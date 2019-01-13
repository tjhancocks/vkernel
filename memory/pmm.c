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

#include <pmm.h>
#include <panic.h>
#include <print.h>
#include <multiboot.h>

////////////////////////////////////////////////////////////////////////////////

extern void *kernel_start;
extern void *kernel_end;

struct pmm_range
{
	uintptr_t start;
	uintptr_t end;
};

static struct 
{
	struct {
		uint32_t count;
		struct {
			uintptr_t *ptr;
			uintptr_t *base;
			uint32_t frames;
			uint32_t available;
			uintptr_t end;
		} stack;
	} frames;
	struct pmm_range kernel_code;
	struct pmm_range kernel_mods;
	struct pmm_range kernel_reserved;
	struct pmm_range bios;
} pmm;

static void pmm_record_frame(uintptr_t frame);

////////////////////////////////////////////////////////////////////////////////

oserr init_physical_memory(struct multiboot_info *mb)
{
	/* The first thing to ensure is that the multiboot information we got 
	   actually contains what we need. If it does not then we're in trouble. */
	if (!(mb->flags & MULTIBOOT_INFO_MEM_MAP)) {
		klogc(serr, "Multiboot information lacks a complete memory map.\n");
		panic(
			"Multiboot Information Incomplete",
			"A complete memory map was missing in the multiboot information.\n"
			"%s:%d", __FILE__, __LINE__
		);
	}

	/* Determine how much physical memory the system has. We can do this by
	   adding up the upper and lower memory totals provided in the multiboot
	   information. This may be incorrect on some systems and may need to be
	   accounted for in the future. */
	pmm.frames.count = (mb->mem_lower + mb->mem_upper) / (FRAME_SIZE >> 10);
	klogc(
		sinfo, "The system has %d frames of physical memory.\n", 
		pmm.frames.count
	);

	/* Record information regarding the kernel code */
	pmm.kernel_code.start = mem_align((uintptr_t)&kernel_start, a_down);
	pmm.kernel_code.end = mem_align((uintptr_t)&kernel_end, a_up);
	klogc(
		sinfo, "The kernel is located in physical memory at %p:%p\n",
		pmm.kernel_code.start, pmm.kernel_code.end
	);

	/* Record information regarding any modules being loaded. We're not
	   concerned with what individual modules are, but rather the range of
	   memory they are occupying. */
	if ((mb->flags & MULTIBOOT_INFO_MODS) && (mb->mods_count > 0)) {
		klogc(sinfo, "Kernel modules were provided via multiboot.\n");
		
		multiboot_module_t *mod = (void *)mb->mods_addr;
		pmm.kernel_mods.start = mod[0].mod_start;
		pmm.kernel_mods.end = mod[0].mod_end;

		for (uint32_t i = 1; i < mb->mods_count; ++i) {
			pmm.kernel_mods.start = MIN(
				pmm.kernel_mods.start, mod[0].mod_start
			);
			pmm.kernel_mods.end = MAX(pmm.kernel_mods.end, mod[0].mod_end);
		}

		pmm.kernel_mods.start = mem_align(pmm.kernel_mods.start, a_down);
		pmm.kernel_mods.end = mem_align(pmm.kernel_mods.end, a_up);

		klogc(
			sinfo, "Kernel modules are located in physical memory at %p:%p\n",
			pmm.kernel_mods.start, pmm.kernel_mods.end
		);
	}
	else {
		/* No kernel modules have been loaded. This must be accounted for
		   otherwise the manager will make incorrect assumptions. */
		panic(
			"Expected Kernel Modules",
			"Failed to find the expected and required kernel modules. Please "
			"make sure they are loaded at boot time."
		);
	}

	/* Determine how much memory should be reserved for kernel working memory.
	   Kernel working memory, or the _kernel heap_ is memory that can be
	   dynamically allocated within the kernel itself. This should not be a
	   lot of memory, and should ideally only be temporary use. */

	/* TODO: Determine if this is needed. */

	/* Record the BIOS memory. This is basically fixed across all systems so
	   this is hardcoded. If this is found to have issues, then the range
	   should be recalculated. */
	pmm.bios.start = 0x00000000;	/* Beginning of Physical Memory */
	pmm.bios.end =   0x00100000;	/* 1MiB */

	/* Setup the stack required for storing available frames. */
	uint32_t per_frame = FRAME_SIZE / sizeof(uintptr_t);
	pmm.frames.stack.frames = (
		(pmm.frames.count / per_frame) + (pmm.frames.count % per_frame ? 1 : 0)
	);
	klogc(
		sinfo, "Available frame stack will require %d frames.\n", 
		pmm.frames.stack.frames
	);
	pmm.frames.stack.base = (uintptr_t *)pmm.kernel_mods.end;
	pmm.frames.stack.ptr = pmm.frames.stack.base;
	pmm.frames.stack.end = (uintptr_t)pmm.frames.stack.base + (
		pmm.frames.stack.frames * FRAME_SIZE
	);
	klogc(
		sinfo, "Available frame stack is located at %p\n", 
		pmm.frames.stack.base
	);

	/* At this point the ranges of physical memory are known. However we 
	   still need to produce a stack of available frames. This stack also
	   needs to account for any memory "holes" that are present. These holes
	   are provided to us via the multiboot information in a memory map. */
	klogc(
		sinfo, 
		"Parsing memory map to assist in determining all available frames.\n"
	);
	struct multiboot_mmap_entry *mmap = (void *)mb->mmap_addr;
	uint32_t mmap_size = mb->mmap_length;
	uint32_t mmap_count = mmap_size / sizeof(*mmap);

	uintptr_t first_avail = pmm.frames.stack.end;
	klogc(sinfo, "First available frame is %p\n", first_avail);
	for (uint32_t i = 0; i < mmap_count; ++i) {
		/* Determine the start of the region and the end of it. */
		uintptr_t start = (uintptr_t)mmap[i].addr;
		uintptr_t end = (uintptr_t)(mmap[i].addr + mmap[i].len);
		
		/* If the region is not usable then skip it. */
		if (mmap[i].type != MULTIBOOT_MEMORY_AVAILABLE) {
			klogc(sinfo, "mmap[%d] %p:%p is not usable.\n", i, start, end);
			continue;
		}

		/* Is the first free frame after the of the region? If so then skip. */
		if (first_avail >= end) {
			klogc(sinfo, "mmap[%d] %p:%p is already used.\n", i, start, end);
			continue;
		}

		/* Is the first free frame before the beginning of the region? */
		start = (first_avail < start) ? start : first_avail;

		/* Record the available frames in the stack */
		klogc(sinfo, "mmap[%d] %p:%p is available for use.\n", i, start, end);
		for (; start < end; start += FRAME_SIZE)
			pmm_record_frame(start);
	}

	/* Confirm that the allocator is working correctly */
	uintptr_t expected = pmm.frames.stack.end;
	uintptr_t allocated = pmm_acquire_frame();
	if (expected == allocated) {
		klogc(sok, "Expected frame %p and got %p\n", expected, allocated);
		if (pmm_release_frame(allocated) == e_ok) {
			klogc(sok, "Frame released as expected.\n");
		}
		else {
			klogc(serr, "Did not release frame as expected.");
		}
	}
	else {
		klogc(serr, "Expected frame %p but got %p\n", expected, allocated);
	}

	/* Make a note of how much memory has been "wired" into it. */
	uintptr_t s = 0, e = 0;
	pmm_frame_range(frame_kernel_wired, &s, &e);
	klogc(sinfo, "Total wired kernel memory is %p:%p\n", s, e);

	/* Reaching this point is a good indication of a successful setup */
	klogc(sok, "Physical Memory Manager initialised successfully.\n");
	return e_ok;
}

////////////////////////////////////////////////////////////////////////////////

void pmm_record_frame(uintptr_t frame)
{
	/* Special case: some memory maps indicate parts of the lower 1MiB are free.
	   We need to ignore these. */
	if (frame < 0x00100000)
		return;

	*pmm.frames.stack.base++ = frame;
	pmm.frames.stack.available++;
}

oserr pmm_release_frame(uintptr_t frame)
{
	/* Check to ensure there is space in the stack. This should never fail. */
	if (pmm.frames.stack.available >= pmm.frames.count) {
		klogc(
			serr, "Available frame stack is full. Unable to release frame %p\n",
			frame
		);
		return e_fail;
	}

	*--pmm.frames.stack.ptr = frame;
	pmm.frames.stack.available++;

	return e_ok;
}

uintptr_t pmm_acquire_frame(void)
{
	/* Check to ensure there are available frames. If there are no available
	   frames then panic. */
	if (pmm.frames.stack.available <= 0) {
		panic(
			"Out of Memory",
			"The system has exhausted all available physical memory.\n"
			"%s:%d", __FILE__, __LINE__
		);
	}

	pmm.frames.stack.available--;
	return *pmm.frames.stack.ptr++;
}

////////////////////////////////////////////////////////////////////////////////

enum frame_purpose pmm_frame_purpose(uintptr_t frame)
{
	if (frame >= pmm.bios.start && frame < pmm.bios.end) {
		return frame_bios;
	}
	else if (frame >= pmm.kernel_code.start && frame < pmm.kernel_code.end) {
		return frame_kernel_code;
	}
	else if (frame >= pmm.kernel_mods.start && frame < pmm.kernel_mods.end) {
		return frame_module;
	}
	else if (frame >= pmm.kernel_code.start && frame < pmm.frames.stack.end) {
		return frame_kernel_wired;
	}
	else {
		return frame_unknown;
	}
}

oserr pmm_frame_range(enum frame_purpose purpose, uintptr_t *s, uintptr_t *e)
{
	switch (purpose) {
		case frame_bios:
			*s = pmm.bios.start;
			*e = pmm.bios.end;
			return e_ok;

		case frame_kernel_code:
			*s = pmm.kernel_code.start;
			*e = pmm.kernel_code.end;
			return e_ok;

		case frame_module:
			*s = pmm.kernel_mods.start;
			*e = pmm.kernel_mods.end;
			return e_ok;

		case frame_kernel_wired:
			*s = pmm.kernel_code.start;
			*e = pmm.frames.stack.end;
			return e_ok;
	}

	/* Reaching this point indicates that the purpose could not be determined */
	return e_fail;
}

oserr pmm_set_frame_purpose(uintptr_t frame, enum frame_purpose purpose)
{
	/* TODO */
}