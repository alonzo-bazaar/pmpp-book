#include<stdio.h>
#include<CL/cl.h>

#define ERROR_INCLUDE_IMPLEMENTATION
#define READFILE_INCLUDE_IMPLEMENTATION
#define OCL_UTILS_INCLUDE_IMPLEMENTATION
#define CONFIG_INCLUDE_IMPLEMENTATION

#include "error.h"    // try_... die_...
#include "readfile.h" // read_entire_file
#include "oclutils.h" // get_device, build_program

#include "config.h"   // ps_config struct

// gonna make the config global, it's just easier
ps_config psc;

// and these motherfuckers, I pass them around way too much
// they are only invalidated (freed) as the program is in its final teardown
// and I don't have any threading in here
// so fuck it, we globall
cl_platform_id plat;
cl_device_id dev;

cl_context context;
cl_command_queue queue;
cl_program program;
cl_kernel kernel;

#define TEST_SCAN_INCLUDE_IMPLMEMENTATION
#include "test-scan.h"

int main(int argc, char** argv) {
	populate_default_config(&psc);
	parse_argv_into_config(argc, argv, &psc);

    cl_int err = CL_SUCCESS;

    // get any gpu device
    dev = get_device(0, 0, CL_DEVICE_TYPE_GPU);
    // query the platform of the gpu device
    try_ret(clGetDeviceInfo(dev, CL_DEVICE_PLATFORM,
                            sizeof(cl_platform_id), &plat, NULL));

    // get context for device and get command queue from context
    try_par(context = clCreateContext(NULL, 1, &dev, NULL, NULL, &err));
    try_par(queue = clCreateCommandQueueWithProperties(context, dev,
                                                       NULL, &err));

    // read program code into memory
    char* program_source = NULL;
    size_t program_size = 0;
    const char* program_file = "scan-bk.cl";
    if(!read_entire_file(program_file, &program_source, &program_size))
        die_fmt("could not read file [%s]", program_file);

    // create and compile program
    try_par(program =\
            clCreateProgramWithSource(context, 1,
                                      (const char **)&program_source,
                                      &program_size, &err));

    // shits itself and dies if program doesn't build correctly
    build_program(program, dev, NULL);

    // get kernel from program
    try_par(kernel = clCreateKernel(program, "bk_in_place", &err));

    puts("starting tests...");
    test_everything();
    puts("starting benchmarks...");
    benchmark_everything();

    // cleanup
    // c side
    free(program_source);

    // opencl side
    try_ret(clReleaseKernel(kernel));
    try_ret(clReleaseProgram(program));
    try_ret(clReleaseCommandQueue(queue));
    try_ret(clReleaseContext(context));

    return 0;
}
