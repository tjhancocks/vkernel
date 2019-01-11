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

#include <heap.h>
#include <pmm.h>
#include <vmm.h>
#include <print.h>
#include <panic.h>

////////////////////////////////////////////////////////////////////////////////

static inline uintptr_t heap_align(uintptr_t address)
{
	return (address + 0x04) & ~(0x3);
}

static inline uintptr_t block_start(struct heap_block *block)
{
	return heap_align((uintptr_t)block + sizeof(*block));
}

static inline uint32_t block_size(uint32_t alloc_size)
{
	return heap_align(alloc_size + heap_align(sizeof(struct heap_block)));
}

////////////////////////////////////////////////////////////////////////////////

oserr init_heap(struct heap **heap, uintptr_t base, uintptr_t limit)
{
	klogc(sinfo, "Constructing heap (base %p, limit %p)\n", base, limit);

	/* Validate everything that has been provided. A heap must be of a certain
	   size, aligned to a page boundary and above a certain point in memory. */
	if (!heap) {
		klogc(serr, "No heap reference provided. Aborting.\n");
		return e_fail;
	}

	if ((base & 0xFFF) || (limit & 0xFFF) || (base >= limit)) {
		klogc(
			serr, 
			"Heap is not aligned (%p:%p). It should be page aligned\n",
			base, limit
		);
		return e_fail;
	}

	/* TODO: Check proposed heap size. */

	/* The heap will require the first page in itself to be allocated. This is
	   so the initial data structures can be constructed (the heap manages its
	   own memory). */
	klogc(sinfo, "Acquiring page for heap: %p\n", base);
	if (vmm_acquire_page(base) != e_ok) {
		klogc(serr, "Failed to acquire initial page for heap.\n");
		return e_fail;
	}

	/* Get a reference to the heap and setup the initial information. */
	klogc(sinfo, "Casting base into heap structure\n");
	*heap = (void *)base;
	(*heap)->base = base;
	(*heap)->limit = limit;
	(*heap)->block_count = 1;
	(*heap)->free_blocks = 1;
	(*heap)->first = (void *)heap_align(base + sizeof(**heap));
	(*heap)->last = (*heap)->first;

	klogc(sinfo, "Heap structure established at %p\n", *heap);
	klogc(sinfo, "Heap first block = %p\n", (*heap)->first);

	/* Setup the first block. */
	(*heap)->first->state = heap_block_free;
	(*heap)->first->size = (limit - block_start((*heap)->first));
	(*heap)->first->owner = *heap;
	(*heap)->first->next = NULL;
	(*heap)->first->back = NULL;

	klogc(
		sinfo, "Heap first block %p is %u bytes in size. First alloc: %p\n", 
		(*heap)->first, (*heap)->first->size, block_start((*heap)->first)
	);

	/* Heap setup and constructed successfully. */
	return e_ok;
}

////////////////////////////////////////////////////////////////////////////////

static oserr heap_map_page(uintptr_t addr)
{
	if (vmm_acquire_page(addr) != e_ok) {
		panic(
			"Heap Allocation Failure",
			"The heap has been unable to correctly acquire memory for "
			"an allocation."
		);
	}
	return e_ok;
}

static oserr heap_map_pages(struct heap_block *block)
{
	uintptr_t base = (uintptr_t)block;
	uintptr_t limit = block_start(block) + block->size;
	for (uintptr_t addr = base; addr < limit; addr += FRAME_SIZE) {
		(void)heap_map_page(addr); /* Ignore result - handled above. */
	}
	return e_ok;
}

////////////////////////////////////////////////////////////////////////////////

void *heap_alloc(struct heap *heap, uint32_t size)
{
	/* Search through the blocks in the heap for a suitable block to use for the
	   requested allocation. When a block is found, it will be divided into
	   2 blocks (if large enough), one being a "free" block and the other
	   being used for the allocation. */
	if (!heap)
		return NULL;

	uint32_t req_block_size = block_size(size);
	klogc(
		sinfo, 
		"Using heap %p. Requested size is %u bytes. Require %u bytes minimum\n",
		heap, size, req_block_size
	);

	struct heap_block *ptr = heap->first;
	do {
		if (ptr->state == heap_block_free) {

			/* Is the block large enough to get an allocation out of? */
			if (ptr->size >= req_block_size) {
				/* We can make a division. First record some of the existing
				   information so that we can update the block list afterward */
				uint32_t orig_size = ptr->size;
				struct heap_block *next = ptr->next;

				/* Allocate this block first */
				ptr->state = heap_block_used;
				ptr->size = heap_align(size);
				ptr->next = (void *)(block_start(ptr) + ptr->size);

				/* Setup the new block */
				heap_map_page((uintptr_t)ptr->next);
				ptr->next->state = heap_block_free;
				ptr->next->next = next;
				ptr->next->back = ptr;
				ptr->next->size = orig_size - (
					(uintptr_t)ptr->next - (uintptr_t)ptr
				);
				ptr->next->owner = heap;
				next->back = ptr->next;

				/* Update the heap. We now have more blocks in the heap.
				   However the free block count stays the same. */
				heap->block_count++;

				/* Return the new block */
				return (void *)block_start(ptr);
			}
			else if (ptr->size >= size) {
				/* We can use the block */
				ptr->state = heap_block_used;
				heap->free_blocks--;
				return (void *)block_start(ptr);
			}

		}
	} while ((ptr = ptr->next) != NULL);

	/* Reaching this point indicates that we failed to find anything. Return
	   NULL to indicate a failed allocation. */
	return NULL;
}

void heap_free(void *ptr)
{
	/* TODO */
}