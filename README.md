# vkernel
vkernel is a subproject of the Veracyon Project. Veracyon is the name of the 
wider Operating System, whereas _vkernel_ is the name of the kernel itself. The
name is short for _veracyon kernel_.

The entire project is a hobbyists endevour and should not be regarded as 
suitable for day to day use (if for no other reason than it doesn't do a whole
lot yet). However it is my aim to continue this project and reach a point in 
which the entire project is self-hosting and could be used for basic day to day
use.

### Goals and Aims
I will not talk about the goals and aims of the wider Veracyon Project, as there
are a couple of differences in its goals (as the wider project includes more
than just a kernel).

_vkernel_ will hopefully be the culmination in much of the knowledge I've learnt
regarding OS development, and be the foundation to much of my future work in 
this area. My hope is to make the kernel portable. I don't mean the same binary
working on multiple architectures, but for much of the code base to be common
between architectures. Currently the kernel only supports the i386 architecture,
but I'm trying to structure the code in such a way that other architectures, 
such as x86_64 or ARM can be added in the future with minimal headaches.

Currently the plan is to produce a "hybrid" kernel. I'm not sure I want to 
pursue a full microkernel design, but I also don't want to end up with a 
monolithic kernel either. Where this balance will end up being, I'm not entirely 
sure yet. It will depend on how development goes.

I also want to support both newer and older features and functionality, with
the kernel attempting to use newer features and then falling back to older ones
if required.

### Plans
There is a lot of work to do in _vkernel_ before it is ready for much of 
anything. There are a few core aspects that need to be added.

- CPU Identification & Topology
- Handling interrupts (not just ACK'ing them)
- Core hardware support (PS/2, ACPI, PCI, etc)
- Ramdisk for essential components not built in to the kernel (filesystems, etc)
- Threading / Multitasking
- Processes
- Symetric Multiprocessing (SMP)
- User-Space
- System Calls
- OS Specific Toolchain
- Security (SSP, etc)

### License (MIT)

```
Copyright (c) 2018-2019 Tom Hancocks

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```
