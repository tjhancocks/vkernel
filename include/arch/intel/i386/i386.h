/*
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
 */

#if !defined(i386_H) && __i386__
#define i386_H

#include <types.h>

#define PAGE_SIZE 0x1000 /* 4KiB Pages */

#include <arch/intel/i386/registers.h>
#include <arch/intel/i386/paging.h>
#include <arch/intel/i386/segments.h>
#include <arch/intel/i386/cpuid.h>
#include <arch/intel/i386/tss.h>

struct i386_cpu
{
	char vendor[16];
	char brand[48];
	enum i386_feature cpuid_features_lo;
	enum i386_feature cpuid_features_hi;
};

extern struct i386_cpu master_cpu;
void init_i386_cpu(struct i386_cpu *cpu);

#endif