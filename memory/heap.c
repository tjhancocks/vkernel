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
	if (vmm_acquire_page(base) != e_ok) {
		klogc(serr, "Failed to acquire initial page for heap.\n");
		return e_fail;
	}

	/* Get a reference to the heap and setup the initial information. */
	*heap = (void *)base;
	(*heap)->base = base;
	(*heap)->limit = limit;
	(*heap)->block_count = 1;
	(*heap)->free_blocks = 1;
	(*heap)->first = (void *)heap_align(base + sizeof(**heap));
	(*heap)->last = (*heap)->first;


	/* Setup the first block. */
	(*heap)->first->state = heap_block_free;
	(*heap)->first->size = (limit - block_start((*heap)->first));
	(*heap)->first->owner = *heap;
	(*heap)->first->next = NULL;
	(*heap)->first->back = NULL;

	/* Heap setup and constructed successfully. */
	return e_ok;
}

////////////////////////////////////////////////////////////////////////////////

static void __debug_dump_heap_block(struct heap_block *block) 
{
	uint32_t block_header_size = heap_align(sizeof(struct heap_block));
	if (block) {
		klog("+ block %p (state=%#08x)\n", block, block->state);
		klog("  - header-size=%d bytes (%#02x)\n", block_header_size, block_header_size);
		klog("  - size=%d bytes (%#02x)\n", block->size, block->size);
		klog("  - start=%p\n", block->start);
		klog("  - next=%p\n", block->next);
		klog("  - prev=%p\n", block->back);
	}
}

static void __debug_dump_heap(struct heap *heap) 
{
	struct heap_block *ptr = heap->first;
	while (ptr) {
		__debug_dump_heap_block(ptr);
		ptr = ptr->next;
	}
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

static oserr heap_unmap_page(uintptr_t addr)
{
	return vmm_release_page(addr);
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

static oserr heap_unmap_pages(struct heap_block *block)
{
	if (block == NULL) {
		klogc(swarn, "Attempted to unmap pages for a NULL block.\n");
		return e_ok;
	}

	/* If the block is the last one in the heap, then we need to act slightly
	   differently. */
	uintptr_t base = block->start;
	uintptr_t limit = 0;
	if (block->next == NULL) {
		/* We need to validate that the upper limit of the heap is page
		   aligned, otherwise we're in danger of potential corruption. */
		limit = block->start + block->size;
		if (limit & (FRAME_SIZE - 1)) {
			/* The end of the heap is not aligned. */
			klogc(swarn, "Heap %p was not correctly aligned!\n", block->owner);
			limit &= ~(FRAME_SIZE - 1);
		}
	}
	else {
		/* Get the start of the next block, and page align down. */
		limit = ((uintptr_t)block->next) & ~(FRAME_SIZE - 1);
	}

	/* We also need to find the actual start. If something else before the block
	   is also using the page, we must not unmap it. */
	if (block->start & (FRAME_SIZE - 1)) {
		/* The block is not aligned to the start of the page. */
		base = (block->start + FRAME_SIZE) & ~(FRAME_SIZE - 1);
	}

	/* We're ready to actually unmap the pages now. */
	if (limit > base) {
		vmm_release_pages(base, limit);
	}

	return e_ok;
}

////////////////////////////////////////////////////////////////////////////////

static void *heap_merge_blocks(struct heap_block *b0, struct heap_block *b1)
{
	/* We're mergine b1 into b0. This should be relatively straight forward. */
	if (b1 <= b0) {
		/* Wrong way around! */
		klogc(
			swarn, "Incorrect block order specified for merge. %p into %p\n",
			b1, b0
		);
		return b0;
	}

	/* Get the details of the block following b1. */
	struct heap_block *b2 = b1->next;

	if (b2) {
		/* There is a block following b1. Use it to perform any calculations
		   required. */
		b0->size = (uintptr_t)b2 - b0->start;
		b0->next = b2;
		b2->back = b0;
	}
	else {
		/* b1 is the terminating block! Slightly more complex, but still 
		   straight forward. */
		b0->next = NULL;
		b0->size += heap_align(sizeof(*b1)) + b1->size;
	}

	/* Update the start pointer */
	b0->start = block_start(b0);

	/* Return b0 to the caller as the new "merged" block */
	return b0;
}

////////////////////////////////////////////////////////////////////////////////

void *heap_alloc(struct heap *heap, uint32_t size)
{
	/* Search through the blocks in the heap for a suitable block to use for the
	   requested allocation. When a block is found, it will be divided into
	   2 blocks (if large enough), one being a "free" block and the other
	   being used for the allocation. */
	if (!heap) {
		klogc(swarn, "Bad allocation attempt on NULL heap.\n");
		return NULL;
	}

	uint32_t req_block_size = block_size(size);

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
				ptr->start = block_start(ptr);
				ptr->next = (void *)(ptr->start + ptr->size);

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

				/* Make sure the required pages are mapped. */
				heap_map_pages(ptr);

				/* Update the heap. We now have more blocks in the heap.
				   However the free block count stays the same. */
				heap->block_count++;

				/* Return the new block */
				return (void *)ptr->start;
			}
			else if (ptr->size >= size) {
				/* We can use the block, but make sure it is valid. */
				ptr->state = heap_block_used;
				ptr->start = block_start(ptr);
				heap->free_blocks--;
				heap_map_pages(ptr);
				return (void *)ptr->start;
			}

		}
	} while ((ptr = ptr->next) != NULL);

	/* Reaching this point indicates that we failed to find anything. Return
	   NULL to indicate a failed allocation. */
	return NULL;
}

void heap_dealloc(struct heap *heap, void *ptr)
{
	uint32_t block_header_size = heap_align(sizeof(struct heap_block));

	if (ptr == NULL) {
		klogc(swarn, "Attempted to deallocate NULL.\n");
		return;
	}
	else if (ptr < heap->base + block_header_size) {
		klogc(swarn, "Attempt to deallocate an invalid pointer %p!\n", ptr);
		__debug_dump_heap(heap);
		return;
	}


	/* Determine the actual block structure for the provided pointer. */
	struct heap_block *block = (uintptr_t)ptr - block_header_size;
	/* Validate that this is the required block. TODO this, check that
	   the owner is the specified heap and that it has a valid start. */
	if (block->owner != heap || block->start != (uintptr_t)ptr) {
		/* Invalid deallocation attempted. Warn and ignore. */
		klogc(swarn, "Attempted to perform invalid deallocation %p.\n", ptr);
		return;
	}

	/* Mark the block as free, and try to collect neighbouring blocks. */
	block->state = heap_block_free;

	/* TODO: Merging blocks together */
	struct heap_block *next = block->next;
	struct heap_block *back = block->back;

	if (back && back->state == heap_block_free) {
		/* Merge block into back */
		block = heap_merge_blocks(back, block);
	}

	if (next && next->state == heap_block_free) {
		/* Merge next into block */
		block = heap_merge_blocks(block, next);
	}

	/* Check the entire range of the block's memory. Can we unmap any pages? */
	heap_unmap_pages(block);
}