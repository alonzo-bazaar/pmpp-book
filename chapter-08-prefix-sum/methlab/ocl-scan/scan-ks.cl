// kogge stone scan, opencl edition

// don't read from local memory rn, just kogge
// we may later use the scan as a bizzarro chapter 4
// does array parameter notation even work in opencl c?
// no, no it doesn't, :(
__kernel void ks_in_place(const uint arrLen,
                          __global float* arr) {
    size_t i = get_global_id(0);
    if(i < arrLen) {
        for(size_t stride = 1; stride < arrLen; stride*=2) {
            barrier(CLK_LOCAL_MEM_FENCE);
            size_t j = i + stride;
            if(j < arrLen)
                arr[j] += arr[i];
        }
    }
}

__kernel void ks(const uint arrLen,
                 __global float* dest, const __global float* src) {

}
