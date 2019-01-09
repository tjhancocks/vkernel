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

#include <arch/intel/intel.h>
#include <serial.h>

struct i386_descriptor master_gdt[GDT_SIZE] 
__attribute__((aligned(PAGE_SIZE))) = {
	[SELECTOR_TO_IDX(KERNEL_CS)] = MAKE_DESCRIPTOR(0, 0xfffff, 0xc, 0x9a),
	[SELECTOR_TO_IDX(KERNEL_DS)] = MAKE_DESCRIPTOR(0, 0xfffff, 0xc, 0x92),
};

struct i386_tss master_tss __attribute__((aligned(PAGE_SIZE))) = {
	.ss0 = 0x10,
	.cs = KERNEL_CS,
	.ss = KERNEL_DS,
	.ds = KERNEL_DS,
	.es = KERNEL_DS,
	.fs = KERNEL_DS,
	.gs = KERNEL_DS,
	.iomap_base = sizeof(struct i386_tss),
};

struct i386_descriptor_pointer master_gdt_ptr;

extern void load_gdt(void *restrict ptr);
static inline void ltr(uint32_t seg)
{
	__asm__ __volatile__(
		  "ltr %0"
		:
		: "rm"((uint16_t)seg)
	);
}

void init_tss(void)
{
	uintptr_t base = (uintptr_t)&master_tss;
	uint32_t limit = (uintptr_t)&master_tss + sizeof(master_tss);

	// Configure the TSS descriptor manually.
	master_gdt[SELECTOR_TO_IDX(TSS_SEG)].limit_lo = limit & 0xffff;
	master_gdt[SELECTOR_TO_IDX(TSS_SEG)].limit_hi = (limit >> 16) & 0xff;
	master_gdt[SELECTOR_TO_IDX(TSS_SEG)].base_lo = base & 0xffff;
	master_gdt[SELECTOR_TO_IDX(TSS_SEG)].base_mid = (base >> 16) & 0xff;
	master_gdt[SELECTOR_TO_IDX(TSS_SEG)].base_hi = (base >> 24) & 0xff;
	master_gdt[SELECTOR_TO_IDX(TSS_SEG)].granularity = 0x00;
	master_gdt[SELECTOR_TO_IDX(TSS_SEG)].access = 0xe9;
}

void init_gdt(void)
{
	init_tss();

	master_gdt_ptr.size = sizeof(master_gdt) - 1;
	master_gdt_ptr.ptr = &master_gdt;
	load_gdt(&master_gdt_ptr);
	ltr(TSS_SEG);
}

#endif