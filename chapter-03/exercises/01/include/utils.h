#ifndef RANDOM_CUDA_UTILS_H
#define RANDOM_CUDA_UTILS_H
#include<cstdio>     // printf, fprintf, stderr, &Co.
#include<sysexits.h>  // for exit failure code
#include<cstdlib>    // for the exit() function

#include<cuda.h>     // for cudaGetErrorString()

// printing
__host__ __device__ void
print_vec(size_t elts, float* vec) {
    putchar('[');
    for(size_t i = 0; i<elts-1; ++i)
	printf("%4.2f, ", vec[i]);

    printf("%4.2f", vec[elts-1]);
    putchar(']');
}

__host__ __device__ void
println_vec(size_t elts, float* vec) {
    print_vec(elts, vec);
    putchar('\n');
}

__host__ __device__ void
print_mat(size_t nrows, size_t ncols, float* mat) {
    if(nrows==1) {
	putchar('['); print_vec(ncols, mat); putchar(']');
	return;
    }

    putchar('[');
    float*row = mat;
    for(size_t i=0; i<nrows-1; i++, row+=ncols) {
	print_vec(ncols, row);
	putchar(','); putchar('\n'); putchar(' ');
    }
    print_vec(ncols, mat);
    putchar(']');
}

__host__ __device__ void
println_mat(size_t nrows, size_t ncols, float* mat) {
    print_mat(nrows, ncols, mat);
    putchar('\n');
}

// error reporting wrapper
void
checking(cudaError_t error, const char* file_name, const int line_num) {
    if(error != cudaSuccess) {
	fprintf(stderr,
		"OH SHIT: we have an error!"
		"\n%s"
		"\nat line [%d]"
		"\nin file [%s]"
		"\nI shall now halt and catching fire"
		"\nthank you for your attention",
		cudaGetErrorString(error),
		line_num,
		file_name);
	exit(EXIT_FAILURE);
    }
}

#define or_die(call) checking(call, __FILE__, __LINE__);

// the user may or may not #define DEBUGGING before including utils.h
#ifdef DEBUGGING
#define DEBUG_PRINTF(fmt, ...) printf("[DEBUG] " fmt, __VA_ARGS__)
#else
#define DEBUG_PRINTF(fmt, ...) (void)0
#endif

#endif //RANDOM_CUDA_UTILS_H
