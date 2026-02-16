#include<stdio.h>
#include<CL/cl.h>

#define CL_STOLEN_UTILS_IMPLEMENTATION
#include "error.h"    // error handling macros (we'll use them a lot)
#include "file.h"     // utilities to read files
#include "context.h"  // wrappers around some opencl api calls

int main() {
    // universally used "errors go here" variables
    cl_int err = CL_SUCCESS;


    cl_platform_id platform_id;
    cl_device_id device_id;
    // get handle to a device
    OCLERROR_PAR(device_id = cl_util_get_device(0, 0, CL_DEVICE_TYPE_GPU,
                                                &err),
                 err,
                 fuck);
    // query for device platform 
    OCLERROR_RET(clGetDeviceInfo(device_id, CL_DEVICE_PLATFORM,
                                 sizeof(cl_platform_id), &platform_id, NULL),
                 err,
                 fuck);
    cl_util_print_device_info(device_id);

    // get context to use device
    cl_context context;
    OCLERROR_PAR(context = clCreateContext(NULL, 1, &device_id,
                                          NULL, NULL, &err),
                 err,
                 fuck);
    // get command queue for the device (requires a context for the device)
    cl_command_queue queue;
    OCLERROR_PAR(queue = clCreateCommandQueueWithProperties(context, device_id,
                                                            NULL, &err),
                 err,
                 context_fuck);

    // now that we have a command queue we can create a kernel
    // and enqueue a command to launch it on the command queue
    // read the program source
    char *program_source = NULL;
    size_t program_size = 0;
    OCLERROR_PAR(program_source =\
                 cl_util_read_exe_relative_text_file("hello.cl",
                                                     &program_size,
                                                     &err),
                 err,
                 queue_fuck);

    // create program
    cl_program program = NULL;
    OCLERROR_PAR(program =\
                 clCreateProgramWithSource(context, 1,
                                           (const char **)&program_source,
                                           &program_size, &err),
                 err,
                 program_source_fuck);

    // compile program
    OCLERROR_RET(cl_util_build_program(program, device_id, NULL),
                 err,
                 program_fuck);

    // create kernel
    // the string in the clCreateKernel call is the name the kernel function
    // has in the opencl program
    cl_kernel kernel = NULL;
    OCLERROR_PAR(kernel = clCreateKernel(program, "vec_add", &err),
                 err,
                 program_fuck);

    // we created the kernel but now need data for the kernel to operate on
    // begin by creating some arrays on the host
    // we will, for programming ease, use an increasing and a constant array
    const size_t vectors_length = 20;
    const size_t vectors_size = vectors_length * sizeof(float);
    // they must be allocated dynamically because of fucking
    // https://stackoverflow.com/questions/20654191/c-stack-memory-goto-and-jump-into-scope-of-identifier-with-variably-modified
    float* host_in1 = (float*)malloc(vectors_size);
    float* host_in2 = (float*)malloc(vectors_size);
    float* host_out = (float*)malloc(vectors_size);
    for(size_t i = 0; i<vectors_length; ++i) {
        host_in1[i] = (float)i;
        host_in2[i] = 10.0f;
    }

    // move those vectors to the host
    // create host buffers to store them in with the data
    cl_mem dev_in1, dev_in2, dev_out;
    OCLERROR_PAR(dev_in1 =\
                 clCreateBuffer(context,
                                CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                vectors_size,
                                host_in1,
                                &err),
                 err,
                 host_buffers_fuck);

    OCLERROR_PAR(dev_in2 =\
                 clCreateBuffer(context,
                                CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                vectors_size,
                                host_in2,
                                &err),
                 err,
                 buffer_1_fuck);

    // we'll also need to allocate an output vector
    // (this one's not in the saxpy example)
    OCLERROR_PAR(dev_out = clCreateBuffer(context,
                                          CL_MEM_WRITE_ONLY,
                                          vectors_size,
                                          NULL,
                                          &err),
                 err,
                 buffer_2_fuck);

    // in opencl we must set every parameter of the kernel individually before
    // launching it
    // (it is indeed as boring as it sounds)
    OCLERROR_RET(clSetKernelArg(kernel, 0, sizeof(cl_mem), &dev_out),
                 err,
                 buffer_out_fuck);
    OCLERROR_RET(clSetKernelArg(kernel, 1, sizeof(cl_mem), &dev_in1),
                 err,
                 buffer_out_fuck);
    OCLERROR_RET(clSetKernelArg(kernel, 2, sizeof(cl_mem), &dev_in2),
                 err,
                 buffer_out_fuck);

    // we launch the kernel by enqueueing it
    OCLERROR_RET(clEnqueueNDRangeKernel
                 (queue, kernel, 1, NULL,
                  &vectors_length, // the amount of work units (as a pointer?)
                  NULL, 0, NULL, NULL), // buncha nulls, thanks khronos
                 err,
                 buffer_out_fuck);

    // get results out
    // when creating queue with clCreateCommandQueueWithProperties we
    // specified no properties
    // one of the properties you can define, and which is off by default, is
    // allowing for out of order execution of the enqueued commands

    // since it's off this means the program will wait for the kernel to
    // finish before performing this read
    OCLERROR_RET(clEnqueueReadBuffer(queue, dev_out, CL_BLOCKING, 0,
                                     vectors_size, (void *)host_out,
                                     0, NULL, NULL),
                 err,
                 buffer_2_fuck);

    // print the result
    for(size_t i = 0; i<vectors_length; ++i) {
        printf("%f ", host_out[i]);
    }
    putchar('\n');

    // cleanup
    cl_int end_err = CL_SUCCESS; // same as err, but for the teardown

    OCLERROR_RET(clReleaseMemObject(dev_out),
                 end_err,
                 buffer_2_fuck);
    OCLERROR_RET(clReleaseMemObject(dev_in2),
                 end_err,
                 buffer_1_fuck);
    free(host_in1);
    free(host_in2);
    free(host_out);
    OCLERROR_RET(clReleaseMemObject(dev_in1),
                 end_err,
                 kernel_fuck);
    OCLERROR_RET(clReleaseKernel(kernel),
                 end_err,
                 program_fuck);
    OCLERROR_RET(clReleaseProgram(program),
                 end_err,
                 program_source_fuck);
    free(program_source);
    OCLERROR_RET(clReleaseCommandQueue(queue),
                 end_err,
                 context_fuck);
    OCLERROR_RET(clReleaseContext(context),
                 end_err,
                 fuck);

    return 0;

    // and in case something got fucked, hic sunt error(fuck) labels
    // they appear here in reverse order wrt to how they are in the code above
    // since the more shit we allocated before getting an error the more shit
    // we gotta clean up before killing everything

    // (probably the best way they had to implement a nested try catch in c)
    // this is the same as the regular cleanup logic
    // but interleaved with profanity
    // (the saxpy example does not separate them, I did because why not)

    // it's broken, fuck! but I gotta free the output buffer I allocated
 buffer_out_fuck:
    OCLERROR_RET(clReleaseMemObject(dev_out),
                 end_err,
                 buffer_2_fuck);

    // it's broken, fuck! but I gotta free the input buffers I allocated
 buffer_2_fuck:
    OCLERROR_RET(clReleaseMemObject(dev_in2),
                 end_err,
                 buffer_1_fuck);

    // it's broken, fuck! but I gotta free the input buffer I allocated
 buffer_1_fuck:
    OCLERROR_RET(clReleaseMemObject(dev_in1),
                 end_err,
                 kernel_fuck);

    // it's broken, fuck! but I gotta free the arrays I allocated on host
 host_buffers_fuck:
    free(host_in1);
    free(host_in2);
    free(host_out);

    // it's broken, fuck! but I gotta free the kernel
 kernel_fuck:
    OCLERROR_RET(clReleaseKernel(kernel),
                 end_err,
                 program_fuck);

    // it's broken, fuck! but I gotta free the opencl program
 program_fuck:
    OCLERROR_RET(clReleaseProgram(program),
                 end_err,
                 program_source_fuck);

    // it's broken, fuck! but I gotta free the program source
 program_source_fuck:
    free(program_source);

    // it's broken, fuck! but I gotta free the queue
 queue_fuck:
    OCLERROR_RET(clReleaseCommandQueue(queue),
                 end_err,
                 context_fuck);

    // it's broken, fuck! but I gotta free the context
 context_fuck:
    OCLERROR_RET(clReleaseContext(context),
                 end_err,
                 fuck);

 fuck: // it's broken, fuck! kill everything
    puts("FUCK");
    cl_util_print_error(err);
    puts("END FUCK");
    cl_util_print_error(end_err);
    return 1;
}
