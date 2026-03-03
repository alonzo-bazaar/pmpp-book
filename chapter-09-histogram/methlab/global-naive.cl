// must be launched with at least max(hist_size, input_size) threads
__kernel void global_naive(const __global uchar* input,
                           __global uint* histbuf, // 32?
                           const uint input_size,
                           const uint hist_size) {
    // zero histogram out
    const uint i = get_global_id(0);
    if(i < hist_size)
        histbuf[i] = 0;

    // barrier so we don't zero out histogram while adding to it
    // https://registry.khronos.org/OpenCL/sdk/3.0/docs/man/html/barrier.html
    barrier(CLK_GLOBAL_MEM_FENCE);

    // this is shit, but let's see if it works
    if(i<input_size)
        atomic_add(&histbuf[input[i]], 1);
}
