// must be launched with at least input_size threads
// since every thread here only handles one element of the data

// local buffers must either be of constant length (ie __local uint buf[256])
// or be passed to the kernel by the caller as local memory objects
// which is what we're gonna do here because idk, seems more general
__kernel void private_histogram_sectioned (const __global uchar* input,
                                           __global uint* global_hist,
                                           __local uint* local_hist,
                                           const uint input_size,
                                           const uint hist_size) {
    const uint gi = get_global_id(0);
    const uint li = get_local_id(0);

    // start and end of which parts of the local histogram are we
    // responsible for zeroing out and flushing
    const uint lh_section_length = ceil((float)hist_size/get_local_size(0));
    const uint lh_start = lh_section_length * li;
    const uint lh_end = min(lh_start + lh_section_length, hist_size);
    // printf("thread %u in block, [%u : %u)\n", li, lh_start, lh_end);

    if(gi < hist_size) global_hist[gi] = 0;
    for(uint i = lh_start; i<lh_end; i++) local_hist[i] = 0;

    barrier(CLK_GLOBAL_MEM_FENCE | CLK_LOCAL_MEM_FENCE);

    // the local id matters little here
    if(gi<input_size)
        atomic_add(&local_hist[input[gi]], 1);

    barrier(CLK_GLOBAL_MEM_FENCE | CLK_LOCAL_MEM_FENCE);

    for(uint i = lh_start; i<lh_end; i++)
        atomic_add(&global_hist[i], local_hist[i]);
}
