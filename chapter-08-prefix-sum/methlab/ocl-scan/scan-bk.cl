__kernel void bk_in_place(const uint arrLen,
                          __global float* arr) {
    const uint i = get_global_id(0);
	if(i < arrLen) {
		// first step
		// to reduce thread divergence, at every step
		// we map the first n threads to the n additions we have to make

		// at step 1 (stride = 1) the first arrLen/2 threads will add 
		// arr[1] += arr[0]; arr[3] += arr[2]; ...
		// n=0, { arr[n+1] += arr[n], n+=2 }

		// at step 2 (stride = 2) the first arrLen/4 threads will add 
		// arr[3] += arr[1]; arr[7] += arr[5];
		// n=1, { arr[n+2] += arr[n], n+=2 }
		for(uint stride = 1; stride<=arrLen; stride *=2) {
            barrier(CLK_LOCAL_MEM_FENCE);
			const uint dest = (i+1)*2*stride -1;
			const uint src = dest-stride;
			if(dest < arrLen)
				arr[dest] += arr[src];
		}

		// second step
		// flip of the first one
		for(uint stride = arrLen/4; stride>0; stride/=2) {
            barrier(CLK_LOCAL_MEM_FENCE);
			const uint src = (i+1)*stride*2 - 1;
			const uint dest = src + stride;
			if(dest < arrLen)
				arr[dest] += arr[src];
		}
	}
}
