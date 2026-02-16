/*
 * from
 * https://registry.khronos.org/OpenCL/specs/3.0-unified/html/OpenCL_C.html
 * Variadic functions are not supported,
 * with the exception of printf and enqueue_kernel.
 * this means
 * 
 * #ifdef DEBUGGING
 * #define DEBUG(...) printf(__VA_ARGS__)
 * #else
 * #define DEBUG(...) (void)0
 * #endif
 * 
 * is not valid in opencl c
 * we will therefore have #ifdef around all the debug prints in here
 * which there will be plenty of
 */

// #define DEBUGGING

/*
 * matrix multiplication: out = A*B
 * A is a [common][width_a] matrix, B is a [height_b][common] matrix
 * matrices are represented in a row major fashion
 * we access them like we would flat vectors by doing (row*width) + col
 *
 * does no if (< length) {} check right now
 * might add them later and see if it changes anything
 *
 * I think to make shit __global it must be like, a pointer, or "on the heap"
 * fixed size params like uint are, to my knowledge, not in global memory,
 * and instead stored in "registers" (at least that's what cuda calls them)
 */
__kernel void matmul_naive(const uint height_A,
						   const uint common,// = width_A = height_B
						   const uint width_B,
						   __global float* out,
						   __global const float* A,
						   __global const float* B) {
    // these are synonims for common, with different names for clarity
    // hopefully optimized out, having two extra variables for work item
    // would be a pain in the ass otherwise
    const size_t width_A = common;
    const size_t height_B  = common;

    // which row/column of 'out' are we responsible for computing?
    const size_t col = get_global_id(0);
    const size_t row = get_global_id(1);

	if(row < height_A && col < width_B) {
		float acc = 0;
		// iterate the row-th row of A and the col-th column of B
		// which are both of size common
		// A[row][i], B[i][col]
		for(size_t i = 0; i<common; ++i) {
			size_t a_ind = (width_A * row) + i; // iterate row of A
			size_t b_ind = (width_B * i) + col; // iterate col of B

			acc += A[a_ind] * B[b_ind];

#ifdef DEBUGGING
			if(row == 0 && col == 10) {
				printf("%zu - %zu : %zu/%zu - %f\n",
					   row, col, i, common, acc);
				printf("%f - %f\n", A[a_ind], B[b_ind]);
				printf("%zu - %zu\n", a_ind, b_ind);
			}
#endif
		}

		size_t out_ind = (row * width_B) + col;

#ifdef DEBUGGING
		printf("%zu - %zu : %f\n", row, col, acc);
#endif

		out[out_ind] = acc;
	}
}
