#pragma once

// this file is included after we define all the global opencl stuff
// so we can just use them here without much worry
// and without passing them around all the time
//
// this is rather filthy but the cleaner alternative is a pain in the ass

#include<CL/cl.h>
#include<assert.h>     // for assert()
#include<string.h>     // for memcpy()
#include<math.h>       // for ceil(), fabs(), and the like

#ifdef TEST_SCAN_INCLUDE_IMPLMEMENTATION
#define PRINTVECS_INCLUDE_IMPLEMENTATION
#define TIMING_INCLUDE_IMPLEMENTATION
#define ERROR_INCLUDE_IMPLEMENTATION
#endif

#include"printvecs.h"  // printing vectors, for debugging
#include"timing.h"     // current_millis(), for benchmarking
#include"error.h"      // error reporting macros

// epsilon for float compairisons
#define EPS (0.00001f)

void cpu_scan_in_place(size_t n, float f[restrict n]);
void cpu_scan(size_t n,
			  float dest[restrict n], const float src[restrict n]);

void kernel_scan_in_place(size_t n, float f[restrict n]);
void kernel_scan(size_t n,
				 float dest[restrict n], const float src[restrict n]);

void fill_with_same(size_t n,
					float dest[restrict n], float val);
void fill_with_iota(size_t n,
					float dest[restrict n],
					float starting_from, float stepping_by);

void assert_iota(size_t n,
				 const float v[restrict n],
				 float starting_from, float stepping_by);
void assert_all_val(size_t n,
					const float v[restrict n], float val);
void assert_vectors_equal(size_t n,
						  const float v1[restrict n], float v2[restrict n]);


// use functions above to test our various implementations of scan
// the functions are iladic as they rely on global variables
// (namely the config struct psc, and the various opencl things)
// to decide what to run and with which parameters
void test_const_input(void (*under_test)(size_t n, float f[restrict n]));
void test_iota_input(void (*under_test)(size_t n, float f[restrict n]));

void test_everything();
void benchmark_everything();

#ifdef TEST_SCAN_INCLUDE_IMPLMEMENTATION
void cpu_scan_in_place(size_t n, float f[n]) {
	for(size_t i = 1; i<n; ++i) {
		f[i] += f[i-1];
	}
}

void cpu_scan(size_t n,
			  float dest[restrict n], const float src[restrict n]) {
	memcpy(dest, src, n);
	cpu_scan_in_place(n, dest);
}

void kernel_scan_in_place(size_t n, float f[restrict n]) {
	cl_int err;
	cl_mem dev_f;

	// non è ne read only ne write only
	// lascio solo host_ptr e vediamo
    try_par(dev_f =\
            clCreateBuffer(context,
                           CL_MEM_COPY_HOST_PTR,
                           n * sizeof(float),
                           f, &err));
	
	unsigned int uint_n = (unsigned int)n; // kernels don't accept size_t
	try_ret(clSetKernelArg(kernel, 0, sizeof(unsigned int), &uint_n));
	try_ret(clSetKernelArg(kernel, 1, sizeof(cl_mem), &dev_f));

	// not really required, could have just done &psc.group_dims
	// but this feels clearer
	size_t group_dims[1]  = {(size_t)psc.group_length};

	// global_dims must be an integer multiple of group_dims
	size_t global_dims[1] = {(size_t)(psc.group_length *
									  ceil((float)psc.array_length /
										   (float)psc.group_length))};

    try_ret(clEnqueueNDRangeKernel
            (queue,
			 kernel,
             1,           // work dim is 1 (array length)
             NULL,        // no global offset
             global_dims, // pointer decay my beloved
             group_dims,

             0,      // we don't wait for any event
             NULL,   // so waiting list is null
             NULL)); // don't emit any event once kernel done


    try_ret(clEnqueueReadBuffer(queue, dev_f, CL_BLOCKING, 0,
                                n * sizeof(float),
                                (void *)f,
                                0, NULL, NULL));

	try_ret(clReleaseMemObject(dev_f));
}

// TODO
void kernel_scan(size_t n,
				 float dest[restrict n], const float src[restrict n]) {
	(void)n;
	(void)dest;
	(void)src;
}

void fill_with_same(size_t n,
					float dest[restrict n], float val) {
	for(size_t i = 0; i<n; ++i) {
		dest[i] = val;
	}
}
void fill_with_iota(size_t n,
					float dest[restrict n],
					float starting_from, float stepping_by) {
	float val = starting_from;
	for(size_t i = 0; i<n; ++i) {
		dest[i] = val;
		val += stepping_by;
	}
}

void assert_all_val(size_t n,
					const float v[restrict n], float val) {
	for(size_t i = 0; i<n; ++i) {
		if(fabs(v[i] - val) > EPS)
			die_fmt("ASSERT ALL VAL: "
					"vector and value not equal at position [%zu]\n\n"
					"vector at that position is: [%f]\n"
					"value is:                   [%f]\n",
					i, v[i], val);
	}
}
void assert_vectors_equal(size_t n,
						  const float v1[restrict n], float v2[restrict n]) {
	for(size_t i = 0; i<n; ++i) {
		if(fabs(v1[i] - v2[i]) > EPS)
			die_fmt("ASSERT VECTORS EQUAL: "
					"vectors not equal at position [%zu]\n\n"
					"vector1 at that position is: [%f]\n"
					"vector2 at that position is: [%f]\n",
					i, v1[i], v2[i]);
	}
}


void assert_iota(size_t n,
				 const float v[restrict n],
				 float starting_from, float stepping_by) {
	float val = starting_from;
	for(size_t i = 0; i<n; ++i) {
		if(fabs(v[i]-val) > EPS)
			die_fmt("ASSERT IOTA: "
					"vector not to iota at position [%zu]\n\n"
					"vector at that position is:   [%f]\n"
					"iota at that position is:     [%f]\n",
					i, v[i], val);


		val += stepping_by;
	}
}
void assert_triangular(size_t n,
					   const float v[restrict n],
					   float starting_from, float stepping_by) {
	float val = starting_from;
	float current_step = val + stepping_by;
	for(size_t i = 0; i<n; ++i) {
		if(fabs(v[i]-val) > EPS)
			die_fmt("ASSERT TRIANGULAR: "
					"vector not to triangular at position [%zu]\n\n"
					"vector at that position is:         [%f]\n"
					"triangular at that position is:     [%f]\n",
					i, v[i], val);

		val += current_step;
		current_step += stepping_by;
	}
}

// see include/config.h for psc structure
// see main.c for why is it available here

// plus scan of constant with value v should be
// iosta starting_from v, stepping_by v
void test_const_input(void (*under_test)(size_t n, float f[restrict n])) {
	size_t n = psc.array_length;
	float* v = (float*)calloc(n, sizeof(float));

	// constant 0
	fill_with_same(n, v, 0.0f);
	under_test(n, v);
	assert_all_val(n, v, 0.0f);
	assert_iota(n, v, 0.0f, 0.0f);

	// constant 1
	fill_with_same(n, v, 1.0f);
	under_test(n, v);
	assert_iota(n, v, 1.0f, 1.0f);

	// constant 2
	fill_with_same(n, v, 2.0f);
	under_test(n, v);
	assert_iota(n, v, 2.0f, 2.0f);

	free(v);
}

// plus scan of iota starting_from i stepping_by j
// shold be triangular starting_from i stepping_by
void test_iota_input(void (*under_test)(size_t n, float f[restrict n])) {
	size_t n = psc.array_length;
	float* v = (float*)calloc(n, sizeof(float));
	
	// iota starting at 0
	fill_with_iota(n, v, 0.0f, 1.0f);
	under_test(n, v);
	assert_triangular(n, v, 0.0f, 1.0f);

	// iota starting at 1
	fill_with_iota(n, v, 1.0f, 1.0f);
	under_test(n, v);
	assert_triangular(n, v, 1.0f, 1.0f);

	// iota starting at 2
	fill_with_iota(n, v, 2.0f, 1.0f);
	under_test(n, v);
	assert_triangular(n, v, 2.0f, 1.0f);


	// iota stepping by 2
	fill_with_iota(n, v, 1.0f, 2.0f);
	under_test(n, v);
	assert_triangular(n, v, 1.0f, 2.0f);

	free(v);
}

void test_everything() {
	puts("starting cpu tests");
	if(psc.also_test_cpu) {
		test_const_input(cpu_scan_in_place);
		test_iota_input(cpu_scan_in_place);
	}

	puts("starting kernel tests");
	test_const_input(kernel_scan_in_place);
	test_iota_input(kernel_scan_in_place);
}

// benchmarks are just
// start timer
// run tests
// end timer
// repeat for all platforms under test (cpu and kernel)
void benchmark_everything() {
	puts("starting benchmarks");
}

#endif // TEST_SCAN_INCLUDE_IMPLMEMENTATION
