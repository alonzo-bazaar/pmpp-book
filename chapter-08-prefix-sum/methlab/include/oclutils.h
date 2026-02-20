#pragma once

#include<stdlib.h>  // malloc, free
#include<stdio.h>   // printf

#include<CL/cl.h>

#include"error.h"   // try_... macros

// to build an opencl program and, if build fails, show compilation errors
cl_int build_program(const cl_program pr, const cl_device_id dev,
                     const char *const opt);

cl_device_id get_device(const cl_uint plat_id,
                        const cl_uint dev_id,
                        const cl_device_type type);

#ifdef OCL_UTILS_INCLUDE_IMPLEMENTATION
#include<stdio.h>  // for file stuff
#include<CL/cl.h>  // for compilation stuff
// taken from opencl's sdk utils, adapted from cl_util_build_program
// build program and show log if build is not successful
cl_int build_program(const cl_program pr, const cl_device_id dev,
                     const char *const opt) {
    cl_int err = clBuildProgram(pr, 1, &dev, opt, NULL, NULL);
    if (err != CL_SUCCESS) {
        // no error handling here as error from build program is more valuable
        char *program_log;
        size_t log_size = 0;
        clGetProgramBuildInfo(pr, dev, CL_PROGRAM_BUILD_LOG, 0, NULL,
                              &log_size);
        if ((program_log = (char *)malloc((log_size + 1) * sizeof(char)))) {
            clGetProgramBuildInfo(pr, dev, CL_PROGRAM_BUILD_LOG, log_size,
                                  program_log, NULL);
            program_log[log_size] = '\0';
            printf("Build log is:\n\n%s\nOptions:\n%s\n\n", program_log, opt);
            free(program_log);
        }
    }
    return err;
}

// taken from opencl's sdk utils, adapted from cl_util_get_device
// get device given platform id, device id, and device type
cl_device_id get_device(const cl_uint plat_id,
                        const cl_uint dev_id,
                        const cl_device_type type) {
    cl_platform_id *platforms;
    cl_uint num_platforms = 0;
    cl_device_id *devices;
    cl_uint num_devices = 0;

    // we must get the platform to look for the device in there
    // start by getting the number of platforms
    try_ret(clGetPlatformIDs(0, NULL, &num_platforms));

    if (plat_id >= num_platforms)
        die_fmt("invalid platform id, platform number [%u]"
                " is greater than number of platforms [%u]",
                plat_id, num_platforms);

    // allocate a buffer for that many platforms
    platforms = (cl_platform_id *)calloc(num_platforms,
                                         sizeof(cl_platform_id));
    if(platforms == NULL) die("most likely we're out of memory");

    // populate that vector with IDs for the available platforms
    try_ret(clGetPlatformIDs(num_platforms, platforms, NULL));

    // we got the platform, now we look for the device on that platform
    // get number of devices on that platform
    try_ret(clGetDeviceIDs(platforms[plat_id], type, 0, NULL, &num_devices));

    if (dev_id >= num_devices)
        die_fmt("invalid device id, device number [%u]"
                " is greater than number of devices for platform [%u]",
                plat_id, num_platforms);

    // allocate a vector for that many devices
    devices = (cl_device_id *)calloc(num_devices,
                                     sizeof(cl_device_id));
    if(devices == NULL) die("most likely we're out of memory");

    // populate that vector with IDs for the available devices
    try_ret(clGetDeviceIDs(platforms[plat_id], type,
                           num_devices, devices, NULL));

    cl_device_id result = devices[dev_id];
    
    free(platforms);
    free(devices);

    return result;
}
#endif // OCL_UTILS_INCLUDE_IMPLEMENTATION
