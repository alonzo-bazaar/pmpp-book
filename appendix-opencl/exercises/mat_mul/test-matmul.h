#pragma once

#include<CL/cl.h>  
#include<assert.h>     // for assert()
#include<string.h>     // for memset()
#include<math.h>       // for ceil()
#include"general.h"    // try_... and die_... macros

// for debug prints
// #define DEBUGGING 
// if you're just gonna dump the entire fucking matrix to stdout
// #define VERBOSE_DEBUGGING 

#ifdef DEBUGGING
#include"printvecs.h"  // printing vectors for debugging
#endif

// debug printf functions (turn to no op if debugging disabled)
#ifdef DEBUGGING 

#define DEBUG_PRINTF(fmt, ...)                  \
    printf("[DEBUG] " fmt "\n", __VA_ARGS__)
#define DEBUG_PRINTF_ERR(fmt, ...)                          \
    fprintf(stderr, "[DEBUG ERROR] " fmt "\n", __VA_ARGS__)
#define DEBUG_PRINT(msg)                        \
    puts("[DEBUG] " msg)
#define DEBUG_PRINT_ERR(msg)                    \
    fputs(stderr, "[DEBUG ERROR] " msg)

#else

#define DEBUG_PRINTF(fmt, ...) (void)0
#define DEBUG_PRINTF_ERR(fmt, ...) (void)0
#define DEBUG_PRINT(msg) (void)0
#define DEBUG_PRINT_ERR(msg) (void)0

#endif

long long millis();

// RIGA PER COOLONNA
// RIGA DI A, COLONNA DI B
// LARGHEZZA DI A (ELEMENTI IN UNA RIGA) = ALTEZZA DI B (ELEMENTI IN COLONNA)
// WIDTH_A = COMMON = HEIGHT_B

// function that calls our kernel and gets the results
// will be used to... test the kernel
void kernel_matmul(const cl_kernel kernel,
                   cl_context context,
                   cl_command_queue queue,
                   const unsigned int height_A,
                   const unsigned int common,
                   const unsigned int width_B,
                   const size_t group_x, // x and y sizes of work groups
                   const size_t group_y,
                   float* out, float* A, float* B);

// ignores kernel, context, queue, and queue
// defined this way only for api consistency with kernel_matmul
void cpu_matmul(const cl_kernel kernel,
                cl_context context,
                cl_command_queue queue,
                const unsigned int height_A,
                const unsigned int common,
                const unsigned int width_B,
                const size_t group_x,
                const size_t group_y,
                float* out, float* A, float* B);

// to avoid code duplication
// declare mm_fun as a function type with a generic enough signature 
// and just make every function under test implement this interface
typedef void(*mm_fun)(const cl_kernel, 
                      cl_context,
                      cl_command_queue,
                      const unsigned int,
                      const unsigned int,
                      const unsigned int,
					  const size_t,
					  const size_t,
                      float*, float*, float* B);

// assert equality between two floating point numbers within a given epsilon
// defined as a macro so that assert prints the name of the function where
// the assertion failed, instead of just
// "assertion failed in function float_asseq"
#define float_asseq(a, b, eps) assert(fabs(a - b) < eps && "floats not equal")

// various tests I'm running
void id_times_random (const cl_kernel kernel,
                      cl_context context,
                      cl_command_queue queue,
                      mm_fun under_test);

void all_ones (const cl_kernel kernel,
               cl_context context,
               cl_command_queue queue,
               mm_fun under_test);

// test entrypoint (calls all tests)
void test_matmul(const cl_kernel kernel,
                 cl_context context,
                 cl_command_queue queue);

// benchmark entrypoint
void benchmark_matmul(const cl_kernel kernel,
                      cl_context context,
                      cl_command_queue queue);

#ifdef STB_ISH_TEST_MATMUL_IMPLEMENTATION
#include<sys/time.h>  // gettimeofday (for millis)
#include<stdlib.h>    // rand (for test_id_times_random)
#include<math.h>      // fabs
#include"error.h"

long long millis() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000LL + tv.tv_usec / 1000;
}

/*
 * using unsigned int becase, going from khronos docs
 * https://registry.khronos.org/OpenCL/sdk/3.0/docs/man/html/restrictions.html
 * 
 * Arguments to kernel functions in a program cannot be declared with the
 * built-in scalar types bool, size_t, ptrdiff_t, intptr_t, and uintptr_t
 * or a struct and/or union that contain fields declared to be one of
 * these built-in scalar types.
 */

// wraps matrix multiplcation kernel call to look like a multiplication of
// host matrices
void kernel_matmul(const cl_kernel kernel,
				   cl_context context,
				   cl_command_queue queue,
				   const unsigned int height_A,
				   const unsigned int common,
				   const unsigned int width_B,
				   const size_t group_x,
				   const size_t group_y,
				   float* out, float* A, float* B) {
    cl_int err;

    const unsigned int width_A = common;
    const unsigned int height_B = common;

    // device buffers (memory objects)
    cl_mem dev_A, dev_B, dev_out;

    // allocate and copy data into the first two
    try_par(dev_A =\
            clCreateBuffer(context,
                           CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                           height_A * width_A * sizeof(float),
                           A, &err));
    try_par(dev_B =\
            clCreateBuffer(context,
                           CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                           height_B * width_B * sizeof(float),
                           B, &err));
    // allocate the third one
    try_par(dev_out =\
            clCreateBuffer(context,
                           CL_MEM_WRITE_ONLY,
                           height_A * width_B * sizeof(float),
                           NULL, &err)); 

    // set kernel parameters before launching it
    try_ret(clSetKernelArg(kernel, 0, sizeof(unsigned int), &height_A));
    try_ret(clSetKernelArg(kernel, 1, sizeof(unsigned int), &common));
    try_ret(clSetKernelArg(kernel, 2, sizeof(unsigned int), &width_B));

    try_ret(clSetKernelArg(kernel, 3, sizeof(cl_mem), &dev_out));
    try_ret(clSetKernelArg(kernel, 4, sizeof(cl_mem), &dev_A));
    try_ret(clSetKernelArg(kernel, 5, sizeof(cl_mem), &dev_B));

    // launch parameters
    // width first so that adjacent threads access adjacent matrix things
	// global dimensions must be evenly divisible by local dimensions
	// ergo the whole ceil() thing
    size_t global_dims[2] = {(size_t)group_x*ceil((float)width_B/group_x),
							 (size_t)group_y*ceil((float)height_A/group_y)};
    size_t local_dims[2]  = {group_x, group_y};

    // kernel launch
    // signature found here
    // https://registry.khronos.org/OpenCL/sdk/3.0/docs/man/html/clEnqueueNDRangeKernel.html
    try_ret(clEnqueueNDRangeKernel
            (queue,
			 kernel,
             2,      // work dim is 2 (2d grid, rows and columns)
             NULL,   // global work offset is not provided, so NULL
             &global_dims[0],
// dimensions of work are width_B * height_A
// one work item per cell of output matrix
             &local_dims[0],
// dimensions of work group/block are group_x * group_y
		     
             0,      // no events in waiting list, so the number of them is 0
             NULL,   // and the list of them is NULL
             NULL)); // and no event is liked to this kernel launch, so NULL

    // get data from kernel/device back into host
    // (expects queue to have been intialized WITHOUT the out of order option)

    // note for future self
    // when you start getting arcane memory errors you should always
    // double check every size parameter you pass to one of these opencl
    // motherfuckers, you probably messed one of em up
    // (they take big-ish formulas with a lot of params and shit, easily fucked)
    try_ret(clEnqueueReadBuffer(queue, dev_out, CL_BLOCKING, 0,
                                height_A * width_B * sizeof(float),
                                (void *)out,
                                0, NULL, NULL));

    try_ret(clReleaseMemObject(dev_A));
    try_ret(clReleaseMemObject(dev_B));
    try_ret(clReleaseMemObject(dev_out));
}

void cpu_matmul(const cl_kernel kernel,
                cl_context context,
                cl_command_queue queue,
                const unsigned int height_A,
                const unsigned int common,
                const unsigned int width_B,
				const size_t group_x,
				const size_t group_y,
                float* out, float* A, float* B) {
    // ignored, kept for api consistency with other functions under test
    (void)kernel;
    (void)context;
    (void)queue;
    (void)group_x;
    (void)group_y;

    const unsigned int width_A  = common; // = height_B

    for(unsigned int row = 0; row<height_A; ++row) {
        for(unsigned int col = 0; col<width_B; ++col) {
            float acc = 0;
            for(unsigned int i = 0; i<common; ++i) {
                unsigned int a_ind = (row * width_A) + i; // iterate row of A
                unsigned int b_ind = (i * width_B) + col; // iterate col of B
                acc += A[a_ind] * B[b_ind];
            }
            unsigned int out_ind = (row * width_B) + col;
            out[out_ind] = acc;
        }
    }
}


// fuck it why not
// (TODO: global config struct instead?)
const unsigned int global_height_A = 200;
const unsigned int global_width_A  = 300;
const unsigned int global_height_B = 400;
const unsigned int global_width_B  = 600;

const unsigned int global_common_dimension = global_width_A;

void test_all_ones(const cl_kernel kernel,
                   cl_context context,
                   cl_command_queue queue,
                   mm_fun under_test) {
    DEBUG_PRINT("TEST ALL ONES");

    const unsigned int height_A = global_height_A;
    const unsigned int width_A  = global_width_A;
    const unsigned int height_B = global_height_B;
    const unsigned int width_B  = global_width_B;

    float* A   = (float*)malloc(height_A * width_A * sizeof(float));
    float* B   = (float*)malloc(height_B * width_B * sizeof(float));
    float* out = (float*)malloc(height_A * width_B * sizeof(float));

    // A and B are all ones
    for(unsigned int i = 0; i<height_A * width_A; ++i) A[i] = 1.0f;
    for(unsigned int i = 0; i<height_B * width_B; ++i) B[i] = 1.0f;

    under_test(kernel, context, queue,
               height_A, width_A, width_B,
			   16, 16, // random values I think look cool
               out, A, B);

#ifdef VERBOSE_DEBUGGING
    println_mat(height_A, width_A, A);
    println_mat(height_B, width_B, B);
    println_mat(height_A, width_B, out);
#endif
    // every element in out should be the summation of common times 1.0f
    for(unsigned int i = 0; i<height_A * width_B; ++i)
        if(fabs(out[i] - (float)global_common_dimension) > 0.0001)
            die_fmt("TEST ALL ONES:"
                    "matrix values [%4.2f] - [%4.2f] not equal at index %u",
                    out[i], (float)global_common_dimension, i);

    free(A);
    free(B);
    free(out);
}

void test_id_times_random(const cl_kernel kernel,
                          cl_context context,
                          cl_command_queue queue,
                          mm_fun under_test) {
    DEBUG_PRINT("TEST ID TIMES RANDOM");

    // id is square
    const unsigned int height_A = global_height_A;
    const unsigned int width_A = global_height_A;

    // times whatever
    const unsigned int height_B = global_height_A;
    const unsigned int width_B = global_width_B;

    float* A   = (float*)malloc(width_A * height_A * sizeof(float));
    float* B   = (float*)malloc(width_B * height_B * sizeof(float));
    float* out = (float*)malloc(width_B * height_B * sizeof(float));

    // set a to an identity matrix
    for(unsigned int row = 0; row<height_A; ++row) {
        for(unsigned int col = 0; col<width_A; ++col) {
            unsigned int ind = (row * width_A) + col;
            if(row == col)
                A[ind] = 1.0f;
            else
                A[ind] = 0.0f;
        }
    }

    // set b to fucking whatever
    srand(5); // make test reproducible 
    for(unsigned int row = 0; row<height_B; ++row) {
        for(unsigned int col = 0; col<width_B; ++col) {
            unsigned int ind = (row * width_B) + col;
            B[ind] = (float)(rand()%100)/100.0f;
        }
    }

    under_test(kernel, context, queue,
               height_A, width_A, width_B,
			   16, 16,
               out, A, B);

    for(unsigned int i = 0; i<height_A * width_B; ++i)
        if(fabs(B[i] - out[i]) > 0.0001)
            die_fmt("TEST ALL ONES:"
                    "matrix values [%4.2f] - [%4.2f] not equal at index %u",
                    out[i], B[i], i);

    free(A);
    free(B);
    free(out);
}

// commenting out cpu benchmarks so I can test various kernels against each other instead
// as cpu tests are very slow in comparison and repeating them sounds like a bad idea

void test_matmul(const cl_kernel kernel,
                 cl_context context,
                 cl_command_queue queue) {
    // DEBUG_PRINT("trying cpu");
    // test_all_ones(kernel, context, queue, cpu_matmul);
    // test_id_times_random(kernel, context, queue, cpu_matmul);

    DEBUG_PRINT("trying kernel");
    test_all_ones(kernel, context, queue, kernel_matmul);
    test_id_times_random(kernel, context, queue, kernel_matmul);
}

void benchmark_matmul(const cl_kernel kernel,
                      cl_context context,
                      cl_command_queue queue) {
    long long t;
    // puts("benchmarking cpu");
    // t = millis();
    // for(int i =0 ; i<20; ++i) {
    //     printf("\r%02d", i);
    //     test_all_ones(kernel, context, queue, cpu_matmul);
    //     test_id_times_random(kernel, context, queue, cpu_matmul);
    // }
    // t = millis()-t;
    // printf("\ntook %lld.%lld seconds for 20 runs\n", t/1000, t%1000);

    puts("benchmarking kernel");
    t = millis();
    for(int i = 0; i<20; ++i) {
        printf("\r%02d", i);
        test_all_ones(kernel, context, queue, kernel_matmul);
        test_id_times_random(kernel, context, queue, kernel_matmul);
    }
    t = millis()-t;
    printf("\ntook %lld.%lld seconds for 20 runs\n", t/1000, t%1000);

    return;
}

#endif // STB_ISH_TEST_MATMUL_IMPLEMENTATION
