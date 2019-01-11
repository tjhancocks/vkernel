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

#if !defined(PHYSICAL_MEMORY_MANAGER_H)
#define PHYSICAL_MEMORY_MANAGER_H

#include <arch.h>
#include <types.h>
#include <multiboot.h>

/* Frames are 4KiB in size */
#define FRAME_SIZE	0x1000

/**
 Physical frame purposes. What is the frame being used for in the system? How
 essential is it for preservation?
 */
enum frame_purpose 
{ 
	frame_unknown,
	frame_available, 
	frame_bios, 
	frame_kernel_code,
	frame_kernel_wired, 
	frame_module, 
	frame_video, 
	frame_context
};

/**
 Memory address alignment behaviour.
 	- a_up		Align address to the next frame boundary.
 	- a_down	Align address to the previous frame boundary.
 */
enum align_behaviour 
{ 
	a_up, 
	a_down 
};

/**
 Used for recording frame purposes. It can keep track of how the frame is being
 used, what its linear address mapping is and in which page directory it was
 originally mapped.
 */
union frame {
	struct {
		uint8_t purpose:4;
		uint8_t unused:4;
	} __attribute__((packed)) s;
	uint8_t v;
};

/* Make sure the above union is the correct size. */
_Static_assert(
	sizeof(union frame) == sizeof(uint8_t), "Incorrect union frame size."
);


/**
 Initialise the Physical Memory Manager, using information provided by the
 specified multiboot information structure.
 */
oserr init_physical_memory(struct multiboot_info *mb);

/**
 Determine what the purpose of the specified frame is. This address _must_ be
 a physical address, not a linear address.
 */
enum frame_purpose pmm_frame_purpose(uintptr_t frame);

/**
 Record the purpose of a frame in the Physical Memory Manager.
 */
oserr pmm_set_frame_purpose(uintptr_t frame, enum frame_purpose purpose);

/**
 Acquire an available frame from the Physical Memory Manager.
 */
uintptr_t pmm_acquire_frame(void);

/**
 Return a frame back to the Physical Memory Manager so that it may become
 available for use again.
 */
oserr pmm_release_frame(uintptr_t frame);

/**
 Retrieve the physical memory range of a particular item.
 */
oserr pmm_frame_range(enum frame_purpose purpose, uintptr_t *s, uintptr_t *e);

/**
 Align specified memory address to a frame boundary.
 */
static inline uintptr_t mem_align(
	uintptr_t address, enum align_behaviour behaviour
) {
	switch (behaviour) {
		case a_up: return (address + FRAME_SIZE) & ~(FRAME_SIZE - 1);
		case a_down: 
		default: return address & ~(FRAME_SIZE - 1);
	}
}

#endif