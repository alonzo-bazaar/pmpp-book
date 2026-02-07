#include<cstdlib>
#include<cuda.h>

#include"utils.h"

// each tred adds two elements
__global__ void
matrix_add_cells(size_t nrows, size_t ncols,
		 float* out, const float* in1, const float* in2) {
    // which cell are we working on?
    size_t col = (blockDim.x * blockIdx.x) + threadIdx.x;
    size_t row = (blockDim.y * blockIdx.y) + threadIdx.y;
    DEBUG_PRINTF("%zu - %zu\n", row, col);

    if((row<nrows) && (col<ncols)) {
	DEBUG_PRINTF("inside %zu - %zu\n", row, col);
	size_t idx = (row * ncols) + col;
	out[idx] = in1[idx] + in2[idx];
    }
}

// each tred adds two rows
__global__ void
matrix_add_rows(size_t nrows, size_t ncols,
		float* out, const float* in1, const float* in2) {
    // which row are we working on?
    // (using x for row because we're gonna spawn a 1d grid for this)
    size_t row = (gridDim.x * blockIdx.x) + threadIdx.x;

    if(row < nrows) {
	for(size_t col = 0; col<ncols; ++col) {
	    size_t idx = (row * ncols) + col;
	    out[idx] = in1[idx] + in2[idx];
	}
    }	
}

// each tred adds two columns
__global__ void
matrix_add_cols(size_t nrows, size_t ncols,
		float* out, const float* in1, const float* in2) {
    // which column are we working on?
    size_t col = (gridDim.x * blockIdx.x) + threadIdx.x;

    if(col < ncols) {
	for(size_t row = 0; row<nrows; ++row) {
	    size_t idx = (row * ncols) + col;
	    out[idx] = in1[idx] + in2[idx];
	}
    }	
}

int main() {
    // input data
    float in1_h[] = {
	1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
    };

    float in2_h[] = {
	0.0f, 0.0f, 0.0f, 2.0f, 2.0f, 2.0f,
	2.0f, 2.0f, 2.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 2.0f, 2.0f, 2.0f,
	2.0f, 2.0f, 2.0f, 0.0f, 0.0f, 0.0f,
    };
    const size_t nrows = 4;
    const size_t ncols = 6;
    const size_t nelts = nrows*ncols;

    float* in1_hp = in1_h;
    float* in2_hp = in2_h;
    float* out_hp = (float*)malloc(nelts*sizeof(float));

    println_mat(nrows, ncols, in1_hp);
    println_mat(nrows, ncols, in2_hp);

    // allocate device vectors and move input data to device
    float* in1_d;
    float* in2_d;
    float* out_d;
    or_die(cudaMalloc((void**)&in1_d, nelts*sizeof(float)));
    or_die(cudaMalloc((void**)&in2_d, nelts*sizeof(float)));
    or_die(cudaMalloc((void**)&out_d, nelts*sizeof(float)));

    or_die(cudaMemcpy(in1_d, in1_hp, nelts*sizeof(float),
		      cudaMemcpyHostToDevice));
    or_die(cudaMemcpy(in2_d, in2_hp, nelts*sizeof(float),
		      cudaMemcpyHostToDevice));
	
#if WHAT_WE_DOIN == ELEMENTS
    DEBUG_PRINTF("doing elements\n");
    dim3 gd(8, 8, 1);
    dim3 bd(ceil(ncols/(float)8), ceil(nrows/(float)8), 1);
    matrix_add_cells<<<gd,bd>>>(nrows, ncols, out_d, in1_d, in2_d);

    or_die(cudaMemcpy(out_hp, out_d, nelts*sizeof(float),
		      cudaMemcpyDeviceToHost));
    println_mat(nrows, ncols, out_hp);

#elif WHAT_WE_DOIN == ROWS
    DEBUG_PRINTF("doing rows\n");
    dim3 gd(64, 1, 1);
    dim3 bd(ceil(ncols/(float)64), 1, 1);
    matrix_add_rows<<<gd,bd>>>(nrows, ncols, out_d, in1_d, in2_d);

    or_die(cudaMemcpy(out_hp, out_d, nelts*sizeof(float),
		      cudaMemcpyDeviceToHost));
    println_mat(nrows, ncols, out_hp);

#elif WHAT_WE_DOIN == COLUMNS
    DEBUG_PRINTF("doing columns\n");
    dim3 gd(64, 1, 1);
    dim3 bd(ceil(ncols/(float)64), 1, 1);
    matrix_add_rows<<<gd,bd>>>(nrows, ncols, out_d, in1_d, in2_d);

    or_die(cudaMemcpy(out_hp, out_d, nelts*sizeof(float),
		      cudaMemcpyDeviceToHost));
    println_mat(nrows, ncols, out_hp);

#else
    #error "WHAT_WE_DOIN not set to a known value"

#endif

    // freeing shit
    // in1 and in2 are allocated on the stack
    // it is thus an error to free them, and I shall therefore not free them
    free(out_hp);
    or_die(cudaFree(in1_d));
    or_die(cudaFree(in2_d));
    or_die(cudaFree(out_d));

    return 0;
}
