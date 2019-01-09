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

#if __i386__

#include <arch.h>
#include <arch/intel/intel.h>
#include <print.h>
#include <debug.h>
#include <heap.h>

struct i386_cpu master_cpu = { 0 };

static void identify_cpu(struct i386_cpu *cpu)
{
	uint32_t reg[4];
	char tmp[128];
	char *p;

	/* Determine the vendor of the CPU. */
	cpuid(0, reg);
	memcpy(&cpu->vendor[0], (char *)&reg[1], 4);
	memcpy(&cpu->vendor[8], (char *)&reg[2], 4);
	memcpy(&cpu->vendor[4], (char *)&reg[3], 4);

	/* Attempt to get a brand string */
	cpuid(0x80000000, reg);
	uint32_t max_ext = reg[0];
	if (max_ext >= 0x80000004) {
		/* The brand string is 48 bytes maximum and must be NUL terminated. */
		cpuid(0x80000002, reg);
		memcpy(&tmp[0], (char *)&reg, 16);
		cpuid(0x80000003, reg);
		memcpy(&tmp[16], (char *)&reg, 16);
		cpuid(0x80000004, reg);
		memcpy(&tmp[32], (char *)&reg, 16);

		for (p = tmp; *p != '\0'; ++p) {
			if (*p != ' ') break;
		}
		memcpy(&cpu->brand, p, sizeof(cpu->brand));
	}

	/* Identify the available features of the CPU. */
	cpuid(1, reg);
	cpu->cpuid_features_lo = reg[3];
	cpu->cpuid_features_hi = reg[2];

	if (cpu->cpuid_features_lo & i386_pae) {
		klogc(swarn, "PAE supported and should be enabled.\n");
	}

	if (cpu->cpuid_features_lo & i386_psn) {
		klogc(sinfo, "CPU serial number is available.\n");
	}

	if (cpu->cpuid_features_lo & i386_msr) {
		klogc(sinfo, "Model specific register functionality available.\n");
	}
}

void init_i386_cpu(struct i386_cpu *cpu)
{
	identify_cpu(cpu);

	/* Setup the CPU. */
	init_gdt();
	init_pic();
	init_idt();
	init_paging();

	/* Setup the kernel heap. We want to allocate a quarter of total physical 
	   memory upto 256MiB to the kernel heap. */
	uint32_t total_kib = phys_total_kib();
	uint32_t heap_kib = (total_kib >> 2);
	heap_kib = (heap_kib >= 262144) ? 262144 : heap_kib;
	klog("System has %dKiB total memory\n", total_kib);
	klog("  kernel heap should use %dKiB \n", heap_kib);
	
	/* TODO: Get this to the kernel context. The context should not be 
	   established here however. */
}

#endif