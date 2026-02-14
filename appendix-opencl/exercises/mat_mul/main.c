#include<stdio.h>
#include<CL/cl.h>
#include<sys/time.h> // getgimeofday(), for current_millis()

#define STB_ISH_UTILS_IMPLEMENTATION
#include "general.h"

#define STB_ISH_TEST_MATMUL_IMPLEMENTATION
#include "test-matmul.h"

long long current_millis() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000LL + tv.tv_usec / 1000;
}

int main() {
    cl_int err = CL_SUCCESS;

    cl_platform_id plat;
    cl_device_id dev;
    // get any gpu device
    dev = get_device(0, 0, CL_DEVICE_TYPE_GPU);
    // query the platform of the gpu device
    try_ret(clGetDeviceInfo(dev, CL_DEVICE_PLATFORM,
                            sizeof(cl_platform_id), &plat, NULL));

    // get context for device and get command queue from context
    cl_context context;
    cl_command_queue queue;
    try_par(context = clCreateContext(NULL, 1, &dev, NULL, NULL, &err));
    try_par(queue = clCreateCommandQueueWithProperties(context, dev,
                                                       NULL, &err));

    // read program code into memory
    char* program_source = NULL;
    size_t program_size = 0;
    const char* program_file = "matmul_naive.cl";
    if(!read_entire_file(program_file, &program_source, &program_size))
        die_fmt("could not read file [%s]", program_file);

    // create and compile program
    cl_program program = NULL;
    try_par(program =\
            clCreateProgramWithSource(context, 1,
                                      (const char **)&program_source,
                                      &program_size, &err));

    // shits itself and dies if program doesn't build correctly
    build_program(program, dev, NULL);

    // get kernel from program
    cl_kernel kernel = NULL;
    try_par(kernel = clCreateKernel(program, "matmul_naive", &err));

    puts("starting tests...");
    test_matmul(kernel, context, queue); // HERE IS WHERE SHIT GOES SOUTH

    puts("starting benchmarks...");
    benchmark_matmul(kernel, context, queue);

    // cleanup
    // c side
    free(program_source);

    // opencl side
    try_ret(clReleaseKernel(kernel));
    try_ret(clReleaseProgram(program));
    try_ret(clReleaseCommandQueue(queue));
    try_ret(clReleaseContext(context));

    return 0;
}
