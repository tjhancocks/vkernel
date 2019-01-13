# The vkernel toolchain
If you are interested in contributing or even just taking a look at vkernel then
you will need to know how to build it. This document will assume that you have a
working version of gcc or clang on your machine. If you do not then please make
sure you have that first.

### What is the toolchain?
The toolchain is a set of tools including, but not limited to, the compiler, 
linker, assembler and debugger. These are all used to build software. In the 
case of vkernel, they are used to build the kernel, by converting all of the 
source code files into a single ELF binary.

The following tools are used:
	
- `i686-elf-gcc` (A host independant version of gcc, aka a cross-compiler)
- `i686-elf-ld` (A host independant version of ld, a linker)
- `nasm` (An assembler)
- `qemu-system-x86_64` (An x86_64 emulator, for testing)

The toolchain for vkernel itself is quite light, though this is likely to change
in the future as more gets added.

### The Target Triplet
The target triplet is used to describe the platform that something is going to
run on. Below are a few examples:

- `x86_64-freebsd`
- `x86_64-linux-gnu`
- `i486-unknown-elf`
- `i686-elf`

The triplet is used to describe the following aspects of the targets host;
machine, vendor and operating system. As a triplet it would look like 
`machine-vendor-operatingsystem`. The vendor is commonly dropped however.

So if we take a look at `x86_64-linux-gnu` we can tell that the machine is 
_x86_64_, there is no vendor and the operating system is _linux-gnu_.

Currently vkernel does not have an OS Specific Toolchain and is using a generic
triplet: `i686-elf`. This simply states that the target machine type is _i686_,
no vendor and an _ELF_ compatible environment. You can change the triplet inside
the [Makefile](https://github.com/tjhancocks/vkernel/blob/master/Makefile),
though I do not recommend doing this unless you have particular need to do so.

### Installing the required toolchain
This section of the guide has been written with _macOS_ in mind.

#### GNU Binutils
...