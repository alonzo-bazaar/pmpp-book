# readapted from https://documen.tician.de/pyopencl/
# I just wanna test a kernel, this is waaaay more than enough
# thank you python

# not needed for code to function
# it just makes the opencl compiler output more shit that I'm probably gonna
# need for development purposes
import os
os.environ['PYOPENCL_COMPILER_OUTPUT']='1'

import numpy as np
import pyopencl as cl
import time

class TimeShit:
    def __init__(self, section_name=None):
        self.t = 0
        self.section_name = section_name

    def __enter__(self):
        self.t = time.time()

    def __exit__(self, *ignoredargs):
        t = time.time()-self.t
        if self.section_name is not None:
            print(f'[{self.section_name}] ', end='')
        print(f'took {t} seconds')

# histogram example
# get the histogram of a into b, where
data = np.fromfile('kjv.txt', dtype=np.uint8) # this should be decently large
# or smaller data because the whole kjv freezes when running a bugged kernel
# data_np = np.reshape(np.stack([np.arange(10, dtype=np.uint8)
#                             for _ in range(20)]),
#                   -1)
# 32, mannaggia se lo metti uint8 e il kernel accetta uint32 poi
# grazialcazzo che l'output ti viene strided a minchia
hist_np=np.zeros((256,), dtype=np.uint32)
# we set the opencl shit up
ctx = cl.create_some_context(interactive=False)
queue = cl.CommandQueue(ctx)

# we create two buffers on the device
mf = cl.mem_flags
data_dev = cl.Buffer(ctx, mf.READ_ONLY | mf.COPY_HOST_PTR, hostbuf=data_np)
hist_dev = cl.Buffer(ctx, mf.WRITE_ONLY, hist_np.nbytes)


naive_hist_prog = None
with open('extremely-naive.cl', 'r') as file:
    src = file.read().strip()
    naive_hist_prog = cl.Program(ctx, src).build()

naive_hist = naive_hist_prog.naive_hist
# prg.hist is a kernel
# the field `hist' of the prg object was created because there's a kernel
# called `hist' in the source code of the program
# classic python shit
with TimeShit("naive histogram"):
    naive_hist(queue,
             data_np.shape,
             None,
             # remaining *args are the shit you're gonna pass to
             # the underlying kernel
             data_dev, hist_dev,
             np.uint32(data_np.shape[0]), np.uint32(hist_np.shape[0]))
    queue.finish()

# we create a program
sectioned_hist_prog = None
with open('sectioned-histogram.cl', 'r') as file:
    src = file.read().strip()
    sectioned_hist_prog = cl.Program(ctx, src).build()

sectioned_hist = sectioned_hist_prog.sectioned_hist
# prg.hist is a kernel
# the field `hist' of the prg object was created because there's a kernel
# called `hist' in the source code of the program
# classic python shit
with TimeShit("sectioned histogram"):
    section_size=16
    global_work_size=(np.uint(np.ceil(data_np.shape[0]/section_size)),)
    sectioned_hist(queue,
             global_work_size,
             None,
             # remaining *args are the shit you're gonna pass to
             # the underlying kernel
             data_dev, hist_dev,
             np.uint32(data_np.shape[0]), np.uint32(hist_np.shape[0]))
    # to make sure timing includes the whole kernel runtime
    # and not just enqueueing the kernel
    # https://registry.khronos.org/OpenCL/sdk/3.0/docs/man/html/clFinish.html 
    queue.finish()

cl.enqueue_copy(queue, hist_np, hist_dev)
# now b should contain the histogram of a, to double check
with TimeShit("numpy histogram"):
    dc, _ = np.histogram(data_np, np.arange(257))

assert np.all(dc == hist_np), "now you fucked up"
