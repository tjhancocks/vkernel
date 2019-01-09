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

#include <frame.h>
#include <arch.h>
#include <string.h>
#include <print.h>
#include <debug.h>
#include <panic.h>

extern void *kernel_start;
extern void *kernel_end;
static uintptr_t kernel_start_addr = (uintptr_t)&kernel_start;
static uintptr_t kernel_end_addr = (uintptr_t)&kernel_end;

struct {
	struct {
		uint32_t frames;
	} system;

	struct {
		uintptr_t start;
		uintptr_t end;
	} kernel;

	struct {
		uintptr_t start;
		uintptr_t end;
	} modules;

	struct {
		uintptr_t *base;
		uintptr_t *ptr;
		uint32_t max;
		uint32_t free;
	} frame_stack;
} phys;

////////////////////////////////////////////////////////////////////////////////

uint32_t phys_total_kib(void)
{
	return ADDR_FOR_PAGE(phys.system.frames) >> 10;
}

////////////////////////////////////////////////////////////////////////////////

uintptr_t phys_kernel_start(void)
{
	return phys.kernel.start;
}

uintptr_t phys_kernel_end(void)
{
	return (phys.kernel.end + PAGE_SIZE) & ~(PAGE_SIZE - 1);
}

uint32_t phys_kernel_frames(void)
{
	return PAGES_FOR_BYTES(phys.kernel.end - phys.kernel.start);
}

uint32_t phys_kernel_size(void)
{
	return phys.kernel.end - phys.kernel.start;
}

////////////////////////////////////////////////////////////////////////////////

uintptr_t phys_modules_start(void)
{
	return phys.modules.start;
}

uintptr_t phys_modules_end(void)
{
	return (phys.modules.end + PAGE_SIZE) & ~(PAGE_SIZE - 1);
}

uint32_t phys_modules_frames(void)
{
	return PAGES_FOR_BYTES(phys.modules.end - phys.modules.start);
}

uint32_t phys_modules_size(void)
{
	return phys.modules.end - phys.modules.start;
}

////////////////////////////////////////////////////////////////////////////////

uintptr_t phys_frame_stack(void)
{
	return (uintptr_t)phys.frame_stack.base;
}

uintptr_t phys_frame_stack_ptr(void)
{
	return (uintptr_t)phys.frame_stack.ptr;
}

uintptr_t phys_frame_stack_end(void)
{
	return  phys_frame_stack() + (phys_frame_stack_max() * PAGE_SIZE);
}

uintptr_t __phys_first_free_frame(void)
{
	return phys_frame_stack_end();
}

uint32_t phys_frame_stack_max(void)
{
	return phys.frame_stack.max;
}

uint32_t phys_frame_stack_free(void)
{
	return phys.frame_stack.free;
}

void phys_push_frame(uintptr_t frame)
{
	*--phys.frame_stack.ptr = frame;
	++phys.frame_stack.free;
}

void phys_push_frames(uintptr_t from, uintptr_t to)
{
	for (uintptr_t f = from, i = 0; f < to; f += PAGE_SIZE, ++i)
		phys_push_frame(f);
}

////////////////////////////////////////////////////////////////////////////////

void *phys_zero_frame(uintptr_t frame)
{
	if (is_paging_enabled()) {
		klogc(swarn, "*** phys_zero_frame: paging is now enabled.\n");
		return zero_page(&kernel_pd, (void *)frame);
	}
	return memset((void *)frame, 0, PAGE_SIZE);
}

void *phys_copy_frame(uintptr_t dst, uintptr_t src)
{
	if (is_paging_enabled()) {
		klogc(swarn, "*** phys_copy_frame: paging is now enabled.\n");
		return copy_page(&kernel_pd, (void *)dst, (void *)src);
	}
	return memcpy((void *)dst, (void *)src, PAGE_SIZE);
}

uint32_t phys_frame_count(void)
{
	return phys.system.frames;
}

////////////////////////////////////////////////////////////////////////////////

void phys_parse_mmap(struct multiboot_info *mb)
{
	struct multiboot_mmap_entry *mmap = (void *)mb->mmap_addr;
	uint32_t mmap_size = mb->mmap_length;
	uint32_t mmap_count = mmap_size / sizeof(*mmap);

	uintptr_t first_free = __phys_first_free_frame();
	for (int i = 0; i < mmap_count; ++i) {
		/* If the region is not usable then skip. */
		if (mmap[i].type != MULTIBOOT_MEMORY_AVAILABLE)
			continue;

		/* Is the first free frame after the end of the region? If so then skip
		   the region. */
		uintptr_t last_frame = (uint32_t)(mmap[i].addr + mmap[i].len);
		if (first_free >= last_frame)
			continue;

		/* Is the first free frame before the region? If so, then pull it
		   forwards to the beginning of the region. */
		if (first_free < (uintptr_t)mmap[i].addr)
			first_free = (uintptr_t)mmap[i].addr;

		/* Calculate the last frame of the region, and push the range of free
		   frames. */
		phys_push_frames(first_free, last_frame);
	}
}

////////////////////////////////////////////////////////////////////////////////

static inline void __swap(uint32_t *a, uint32_t *b)
{
	uint32_t tmp = *a;
	*a = *b;
	*b = tmp;
}

static void __reverse_frame_stack(void)
{
	uint32_t half = (phys.frame_stack.free >> 1);
	for (int i = 0; i < half; ++i) {
		__swap(
			&phys.frame_stack.ptr[i], 
			&phys.frame_stack.ptr[phys.frame_stack.free - i - 1]
		);
	}
}

////////////////////////////////////////////////////////////////////////////////

static inline uintptr_t max_address(uintptr_t a, uintptr_t b)
{
	return (a > b) ? a : b;
}

////////////////////////////////////////////////////////////////////////////////

void init_phys(struct multiboot_info *mb)
{
	/* Record physical system properties */
	phys.system.frames = PAGES_FOR_KIB(mb->mem_lower + mb->mem_upper);

	/* Record physical kernel properties */
	phys.kernel.start = kernel_start_addr;
	phys.kernel.end = kernel_end_addr;

	/* Check for the presence of any modules loaded by the bootloader. We need
	   to do some work here to determine how much memory they are actually
	   taking up, as we need to factor this in to our preparations. */
	phys.modules.end = phys_kernel_end();
	if (mb->flags & MULTIBOOT_INFO_MODS) {
		if (mb->mods_count > 0) {
			multiboot_module_t *mod = (void *)(
				phys.modules.start = mb->mods_addr
			);
			uint32_t mod_count = mb->mods_count;

			for (uint32_t i = 0; i < mod_count; ++i) {
				phys.modules.end = max_address(
					phys.modules.end, mod[i].mod_end
				);
			}
		}
	}

	/* Setup the physical frame stack */
	phys.frame_stack.base = (void *)phys_modules_end();
	phys.frame_stack.max = (
		(phys.system.frames / (PAGE_SIZE / sizeof(uint32_t))) + 1
	);
	phys.frame_stack.ptr = (void *)(
		(uintptr_t)phys.frame_stack.base + (phys.frame_stack.max * PAGE_SIZE)
	);
	phys.frame_stack.free = 0;

	/* Prepare for usage and populate the frame stack. */
	phys_parse_mmap(mb);
	__reverse_frame_stack();
}

////////////////////////////////////////////////////////////////////////////////

uintptr_t phys_alloc_frame(void)
{
	if (phys.frame_stack.free == 0) {
		panic(
			"Out of Physical Memory",
			"The system has exhaused all physical memory.\n"
			"This is a bug in the kernel, as it needs to be able to perform "
			"clean up/collection of unused memory."
		);
	}
	--phys.frame_stack.free;
	return *++phys.frame_stack.ptr;
}