#pragma once

// OpenCL includes
#include <CL/cl.h>

// OpenCL Utils includes
#include "error.h"

// STL includes
#include <stdlib.h> // malloc, free
#include <stdio.h>  // printf
#include <time.h>

cl_context cl_util_get_context(const cl_uint plat_id, const cl_uint dev_id,
                               const cl_device_type type, cl_int* const error);
cl_device_id cl_util_get_device(const cl_uint plat_id, const cl_uint dev_id,
                                const cl_device_type type, cl_int* const error);

cl_int cl_util_print_device_info(const cl_device_id device);

char* cl_util_get_device_info(const cl_device_id device,
                              const cl_device_info info, cl_int* const error);
char* cl_util_get_platform_info(const cl_platform_id platform,
                                const cl_platform_info info,
                                cl_int* const error);

// build program and show log if build is not successful
cl_int cl_util_build_program(const cl_program pr, const cl_device_id dev,
                             const char* const opt);

#define GET_CURRENT_TIMER(time)                 \
    struct timespec time;                       \
    timespec_get(&time, TIME_UTC);              \
    {                                           \
    }

#define TIMER_DIFFERENCE(dt, time1, time2)              \
    {                                                   \
        dt = (time2.tv_sec - time1.tv_sec) * 1000000000 \
            + (time2.tv_nsec - time1.tv_nsec);          \
    }

#define START_TIMER GET_CURRENT_TIMER(start_timer1)
#define STOP_TIMER(dt)                                  \
    GET_CURRENT_TIMER(stop_timer2)                      \
        TIMER_DIFFERENCE(dt, start_timer1, stop_timer2)

#ifdef CL_STOLEN_UTILS_IMPLEMENTATION
cl_context cl_util_get_context(const cl_uint plat_id, const cl_uint dev_id,
                               const cl_device_type type, cl_int *const error)
{
    cl_int err = CL_SUCCESS;
    cl_context result = NULL;
    cl_platform_id *platforms;
    cl_uint num_platforms = 0;
    cl_device_id *devices;
    cl_uint num_devices = 0;

    OCLERROR_RET(clGetPlatformIDs(0, NULL, &num_platforms), err, end);
    MEM_CHECK(platforms = (cl_platform_id *)malloc(sizeof(cl_platform_id)
                                                   * num_platforms),
              err, end);
    OCLERROR_RET(clGetPlatformIDs(num_platforms, platforms, NULL), err, plat);

    if (plat_id >= num_platforms)
        {
            fprintf(stderr,
                    "Invalid platform index provided for cl_util_get_context()\n");
            err = CL_UTIL_INDEX_OUT_OF_RANGE;
            goto plat;
        }

    OCLERROR_RET(
                 clGetDeviceIDs(platforms[plat_id], type, 0, NULL, &num_devices), err,
                 plat);
    MEM_CHECK(devices =
              (cl_device_id *)malloc(sizeof(cl_device_id) * num_devices),
              err, plat);
    OCLERROR_RET(
                 clGetDeviceIDs(platforms[plat_id], type, num_devices, devices, NULL),
                 err, dev);

    if (dev_id >= num_devices)
        {
            fprintf(stderr,
                    "Invalid device index provided for cl_util_get_context()\n");
            err = CL_UTIL_INDEX_OUT_OF_RANGE;
            goto dev;
        }

    OCLERROR_PAR(
                 result = clCreateContext(NULL, 1, devices + dev_id, NULL, NULL, &err),
                 err, dev);

 dev:
    free(devices);
 plat:
    free(platforms);
 end:
    if (error != NULL) *error = err;
    return result;
}

cl_device_id cl_util_get_device(const cl_uint plat_id, const cl_uint dev_id,
                                const cl_device_type type, cl_int *const error)
{
    cl_int err = CL_SUCCESS;
    cl_device_id result = NULL;
    cl_platform_id *platforms;
    cl_uint num_platforms = 0;
    cl_device_id *devices;
    cl_uint num_devices = 0;

    // get number of platforms
    OCLERROR_RET(clGetPlatformIDs(0, NULL, &num_platforms), err, end);
    // allocate that many platforms
    MEM_CHECK(platforms = (cl_platform_id *)malloc(sizeof(cl_platform_id)
                                                   * num_platforms),
              err, end);
    // populate the vector you just allocated
    OCLERROR_RET(clGetPlatformIDs(num_platforms, platforms, NULL), err, plat);

    if (plat_id >= num_platforms)
        {
            fprintf(stderr,
                    "Invalid platform index provided for cl_util_get_context()\n");
            err = CL_UTIL_INDEX_OUT_OF_RANGE;
            goto plat;
        }

    OCLERROR_RET(
                 clGetDeviceIDs(platforms[plat_id], type, 0, NULL, &num_devices), err,
                 plat);
    MEM_CHECK(devices =
              (cl_device_id *)malloc(sizeof(cl_device_id) * num_devices),
              err, plat);
    OCLERROR_RET(
                 clGetDeviceIDs(platforms[plat_id], type, num_devices, devices, NULL),
                 err, dev);

    if (dev_id >= num_devices)
        {
            fprintf(stderr,
                    "Invalid device index provided for cl_util_get_context()\n");
            err = CL_UTIL_INDEX_OUT_OF_RANGE;
            goto dev;
        }

    result = devices[dev_id];

 dev:
    free(devices);
 plat:
    free(platforms);
 end:
    if (error != NULL) *error = err;
    return result;
}

cl_int cl_util_print_device_info(const cl_device_id device)
{
    cl_int error = CL_SUCCESS;
    char *name = NULL;

    cl_platform_id platform;
    OCLERROR_PAR(clGetDeviceInfo(device, CL_DEVICE_PLATFORM,
                                 sizeof(cl_platform_id), &platform, NULL),
                 error, nam);

    OCLERROR_PAR(
                 name = cl_util_get_platform_info(platform, CL_PLATFORM_VENDOR, &error),
                 error, ven);
    printf("Selected platform by %s\n", name);

 ven:
    free(name);
    OCLERROR_PAR(name = cl_util_get_device_info(device, CL_DEVICE_NAME, &error),
                 error, ocl);
    printf("Selected device: %s\n", name);

 ocl:
    free(name);
    OCLERROR_PAR(name = cl_util_get_device_info(
                                                device, CL_DEVICE_OPENCL_C_VERSION, &error),
                 error, nam);
    printf("%s\n\n", name);

 nam:
    free(name);
    return error;
}

char *cl_util_get_platform_info(const cl_platform_id platform,
                                const cl_platform_info info,
                                cl_int *const error)
{
    cl_int err = CL_SUCCESS;
    char *name = NULL;
    size_t n = 0;

    switch (info)
        {
        case CL_PLATFORM_PROFILE:
        case CL_PLATFORM_VERSION:
        case CL_PLATFORM_NAME:
        case CL_PLATFORM_VENDOR:
        case CL_PLATFORM_EXTENSIONS:
            OCLERROR_RET(clGetPlatformInfo(platform, info, 0, NULL, &n), err,
                         end);
            MEM_CHECK(name = (char *)malloc(sizeof(char) * (n + 1)), err, end);
            OCLERROR_RET(
                         clGetPlatformInfo(platform, info, sizeof(char) * n, name, NULL),
                         err, end);
            name[n] = '\0';
            if (error != NULL) *error = err;
            return name;
        }

 end:
    free(name);
    if (error != NULL) *error = err;
    return NULL;
}

char *cl_util_get_device_info(const cl_device_id device,
                              const cl_device_info info, cl_int *const error)
{
    cl_int err = CL_SUCCESS;
    char *name = NULL;
    size_t n = 0;

    switch (info)
        {
        case CL_DEVICE_EXTENSIONS:
        case CL_DEVICE_NAME:
        case CL_DEVICE_VENDOR:
        case CL_DEVICE_PROFILE:
        case CL_DEVICE_VERSION:
#ifdef CL_VERSION_1_1
        case CL_DEVICE_OPENCL_C_VERSION:
#endif
#ifdef CL_VERSION_1_2
        case CL_DEVICE_BUILT_IN_KERNELS:
#endif
#ifdef CL_VERSION_2_1
        case CL_DEVICE_IL_VERSION:
#endif
#ifdef CL_VERSION_3_0
        case CL_DEVICE_LATEST_CONFORMANCE_VERSION_PASSED:
#endif
        case CL_DRIVER_VERSION:
            OCLERROR_RET(clGetDeviceInfo(device, info, 0, NULL, &n), err, end);
            MEM_CHECK(name = (char *)malloc(sizeof(char) * (n + 1)), err, end);
            OCLERROR_RET(
                         clGetDeviceInfo(device, info, sizeof(char) * n, name, NULL),
                         err, end);
            name[n] = '\0';
            if (error != NULL) *error = err;
            return name;
        }

 end:
    free(name);
    if (error != NULL) *error = err;
    return NULL;
}

// build program and show log if build is not successful
cl_int build_program(const cl_program pr, const cl_device_id dev,
                     const char *const opt)
{
    cl_int err = clBuildProgram(pr, 1, &dev, opt, NULL, NULL);
    if (err != CL_SUCCESS)
        { // no error handling here as error from build program is more valuable
            char *program_log;
            size_t log_size = 0;
            clGetProgramBuildInfo(pr, dev, CL_PROGRAM_BUILD_LOG, 0, NULL,
                                  &log_size);
            if ((program_log = (char *)malloc((log_size + 1) * sizeof(char))))
                {
                    clGetProgramBuildInfo(pr, dev, CL_PROGRAM_BUILD_LOG, log_size,
                                          program_log, NULL);
                    program_log[log_size] = '\0';
                    printf("Build log is:\n\n%s\nOptions:\n%s\n\n", program_log, opt);
                    free(program_log);
                }
        }
    return err;
}
#endif // CL_STOLEN_UTILS_IMPLEMENTATION
