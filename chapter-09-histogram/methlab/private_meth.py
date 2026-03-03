# makes opencl compiler output more errors
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

data_np=np.fromfile('kjv.txt', dtype=np.uint8)
# data_np = np.reshape(np.stack([np.arange(256, dtype=np.uint8)
#                                for _ in range(400)]),
#                      -1)
hist_np=np.zeros((256,), dtype=np.uint32)

ctx = cl.create_some_context(interactive=False)
queue = cl.CommandQueue(ctx)

mf = cl.mem_flags
data_dev = cl.Buffer(ctx, mf.READ_ONLY | mf.COPY_HOST_PTR, hostbuf=data_np)
hist_dev = cl.Buffer(ctx, mf.WRITE_ONLY, hist_np.nbytes)

# cpu baseline (and double check value)
with TimeShit("numpy histogram"):
    dc, _ = np.histogram(data_np, np.arange(257))

pn_prog = None
with open('private-naive.cl', 'r') as file:
    src = file.read().strip()
    pn_prog = cl.Program(ctx, src).build()

pn_hist = pn_prog.private_naive
with TimeShit("privatized naive histogram"):
    local_work_size = np.uint(256)
    global_work_size = np.uint(local_work_size*
                               np.ceil(data_np.shape[0]/local_work_size))
    print(f'with local group size {local_work_size}')
    print(f'with global group size {global_work_size}')
    pn_hist(queue,
            (global_work_size,),
            (local_work_size,),
            data_dev, hist_dev,
            cl.LocalMemory(256 * 4), # sizeof(uint)=4, local_hist is uint[256]
            np.uint32(data_np.shape[0]), np.uint32(hist_np.shape[0]))
    queue.finish()

cl.enqueue_copy(queue, hist_np, hist_dev)
assert np.all(dc == hist_np), "now you fucked up (naive)"

# one work item per data item
# but work groups may be smaller than the histogram's size
# this means a single thread may be responsible for flushing more than one
# local histogram element to global memory
phs_prog = None
with open('private-histogram-sectioned.cl', 'r') as file:
    src = file.read().strip()
    phs_prog = cl.Program(ctx, src).build()

phs_hist = phs_prog.private_histogram_sectioned
with TimeShit("privatized naive with sectioned histogram"):
    local_work_size = np.uint(256)
    global_work_size = np.uint(local_work_size*
                               np.ceil(data_np.shape[0]/local_work_size))
    print(f'with local group size {local_work_size}')
    print(f'with global group size {global_work_size}')
    phs_hist(queue,
             (global_work_size,),
             (local_work_size,),
             data_dev, hist_dev,
             cl.LocalMemory(256 * 4), 
             np.uint32(data_np.shape[0]), np.uint32(hist_np.shape[0]))
    queue.finish()

cl.enqueue_copy(queue, hist_np, hist_dev)
assert np.all(dc == hist_np), "now you fucked up (sectioned histogram)"

# with TimeShit("privatized sectioned histogram"):
#     section_size=16
#     global_work_size=(np.uint(np.ceil(data_np.shape[0]/section_size)),)
#     sectioned_hist(queue,
#              global_work_size,
#              None,
#              data_dev, hist_dev, LocalMemory(256 * 4), # 4 = sizeof uint
#              np.uint32(data_np.shape[0]), np.uint32(hist_np.shape[0]))
#     queue.finish()

