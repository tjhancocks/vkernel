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
#include <arch.h>
#include <print.h>
#include <debug.h>

#define MAXIMUM_ALLOC_ATTEMPTS	2
struct heap kernel_heap = { 0 };

////////////////////////////////////////////////////////////////////////////////

static void *__heap_alloc_pages(struct heap *heap, uint32_t count)
{
	uintptr_t addr = find_linear_address(
		heap->page_ctx, 0x00400000, 0xFFFFFFFF, count
	);
	if (addr == INVALID_LINEAR_ADDRESS_ERROR) {
		klogc(swarn, "*** Failed to perform heap allocation\n");
		return NULL;
	}
	void *ptr = addr;

	/* Ensure all pages are allocated. */
	for (uint32_t i = 0; i < count; ++i) {
		void *page = alloc_page(heap->page_ctx, (void *)(addr), NULL);
		page = zero_page(heap->page_ctx, page);
		addr += PAGE_SIZE;
	}

	return ptr;
}

struct heap_container *__heap_alloc_container(struct heap *heap)
{
	struct heap_container *container = (void *)__heap_alloc_pages(heap, 1);

	/* Configure the container to make sure it behaves correctly. */
	container->next_link = NULL;
	container->prev_link = NULL;
	container->blocks = (void *)((uintptr_t)(container) + (sizeof(void *) * 3));

	return container;
}

////////////////////////////////////////////////////////////////////////////////

void init_heap(
	struct heap *heap, struct paging_context *ctx, uint32_t size_kib
) {
	/* Setup the provided heap structure. */
	if (!heap) {
		klogc(swarn, "*** Expected a heap. Got NULL.\n");
		return;
	}

	/* We need to ensure we keep track of the appropriate paging context for 
	   this heap, how many pages it uses and where they start. We need this
	   information so that we can destroy the heap in the future if required. */
	heap->page_ctx = ctx;
	heap->pages = PAGES_FOR_KIB(size_kib);
	heap->base = (uintptr_t)__heap_alloc_pages(heap, heap->pages);

	/* Each container will contain a specific number of blocks. This can vary
	   from architecture to architecture so we need to calculate it here rather
	   than hard code it. Thankfully this only needs to be done once.
	   Each container will be a single page in size. */
	heap->container_blocks = (
		(PAGE_SIZE - (sizeof(void*) * 3)) / sizeof(struct heap_block)
	);

	/* We now need to allocate the first container. Containers are used to track
	   "blocks", each container tracking the previously mentioned number of 
	   blocks. When more blocks are needed a new container is created and a 
	   linked list of containers produced. */
	heap->first_container = __heap_alloc_container(heap);
	heap->last_container = heap->first_container;

	/* And configure the first block to act as "free" space. */
	heap->first_container->blocks[0].flags = heap_block_present;
	heap->first_container->blocks[0].offset = heap->base;
	heap->first_container->blocks[0].size = (heap->pages * PAGE_SIZE);

	/* At this point the basic heap should be set up and ready for use. */
}

////////////////////////////////////////////////////////////////////////////////

void describe_heap(struct heap *heap)
{
	klog(">> describe heap: %p\n", heap);
	klog("   paging context: %p\n", heap->page_ctx);
	klog("   pages: %d\n", heap->pages);
	klog("   base: %p\n", heap->base);
	klog("   blocks per container: %d\n", heap->container_blocks);
	klog("   first container: %p\n", heap->first_container);
	klog("   last container: %p\n", heap->last_container);

	struct heap_container *ptr = heap->first_container;
	do {
		klog("   + container: %p (blocks: %p)\n", ptr, ptr->blocks);
		for (int i = 0; i < heap->container_blocks; ++i) {
			struct heap_block *blk = &ptr->blocks[i];
			klog(
				"   |- block %d: %p (%d bytes) %s/%s\n",
				i, blk->offset, blk->size,
				(blk->flags & heap_block_allocated) ? "allocated" : "free",
				(blk->flags & heap_block_present) ? "present" : "missing"
			);
		}
	}
	while ((ptr = ptr->next_link) != NULL);
}

////////////////////////////////////////////////////////////////////////////////

int __chk_heap_block_wants_coalesce(struct heap_block *blk)
{
	return (blk->flags & heap_block_present) 
		&& !(blk->flags & heap_block_allocated);
}

void heap_coalesce(struct heap *heap)
{
	/* TODO: This is a naive implementation of a heap coalescing function, and
	   is in the order of O(N^2) in the worst case scenario. This is fine for
	   short term, and over small collections of blocks, but is not good for use
	   across an entire heap when it is large, or for frequent use in a fully
	   developed/fleshed out kernel. */
	struct heap_container *ptr = heap->first_container;
	do {

		for (int i = 0; i < heap->container_blocks - 1; ++i) {
			struct heap_block *blk = &ptr->blocks[i];

			/* Are we present and not allocated? As in do we have free bytes
			   associated to the block? If not then skip to the next block. */
			if (!__chk_heap_block_wants_coalesce(blk)) {
				continue;
			}

			/* Ensure we actually have some size. If not then make sure we're 
			   not marked as allocated or present. */
			if (blk->size == 0) {
				blk->flags = heap_block_unused;
				continue;
			}

			/* Scan forward for a block/allocation that is next to our current
			   block. */
			for (int j = i + 1; j < heap->container_blocks; ++j) {
				struct heap_block *sblk = &ptr->blocks[j];

				/* Check that the subject block is not allocated and fits the
				   same criteria as above. */
				if (!__chk_heap_block_wants_coalesce(sblk)) {
					continue;
				}

				/* If its an invalid size, then skip. We'll catch it later. */
				if (sblk->size == 0) {
					continue;
				}

				/* Check for the valid scenarios. The allocations must reside 
				   directly next to each other for this to be a valid 
				   operation. */
				if (sblk->offset == (blk->offset - sblk->size)) {
					sblk->size += blk->size;
					blk->flags = heap_block_unused;
					blk->size = 0;
				}
				else if (sblk->offset == (blk->offset + blk->size)) {
					blk->size += sblk->size;
					sblk->flags = heap_block_unused;
					sblk->size = 0;
				}
			}
		}

	}
	while ((ptr = ptr->next_link) != NULL);
}

////////////////////////////////////////////////////////////////////////////////

void *heap_alloc(struct heap *heap, uint32_t size)
{
	uintptr_t allocation_offset = 0;

	for (int attempt = 1; attempt <= MAXIMUM_ALLOC_ATTEMPTS; ++attempt)
	{
		struct heap_block *avail_blk = NULL;
		uint32_t avail_blk_count = 0;

		struct heap_container *ptr = heap->first_container;
		do {
			for (int i = 0; i < heap->container_blocks; ++i) {
				struct heap_block *blk = &ptr->blocks[i];

				/* If we've found an available block and we haven't already 
				   found one record it. If we've already found an allocation  
				   point, then use this missing block for it. */
				if (!(blk->flags & heap_block_present)) {
					if (allocation_offset) {
						blk->flags |= heap_block_allocated | heap_block_present;
						blk->size = size;
						blk->offset = allocation_offset;
						return (void *)blk->offset;
					}
					
					if (!avail_blk) {
						avail_blk = ptr;
					}
					++avail_blk_count;
				}

				/* Make sure we don't have an allocation in progress before
				   continuing */
				if (allocation_offset) {
					continue;
				}

				/* Ignore already allocated blocks */
				if (blk->flags & heap_block_allocated) 
					continue;

				/* Ignore blocks that are not big enough */
				if (blk->size < size)
					continue;

				/* Found a block of space big enough for the allocation.
				   Record where the allocation should be and then adjust the 
				   free space accordingly. */
				allocation_offset = blk->offset;
				blk->size -= size;
				blk->offset += size;

				if (avail_blk) {
					avail_blk->flags |= (
						heap_block_allocated | heap_block_present
					);
					avail_blk->size = size;
					avail_blk->offset = allocation_offset;
					return (void *)avail_blk->offset;
				}
			}
		}
		while ((ptr = ptr->next_link) != NULL);

		/* Failed to find an allocation or available block. Attempt to resolve
		   this issue. If an allocation_offset has been found, then we simply
		   need more blocks to record the allocation. */
		if (allocation_offset && avail_blk_count == 0) {
			struct heap_container *container = __heap_alloc_container(heap);
			heap->last_container->next_link = container;
			container->prev_link = heap->last_container;
			heap->last_container = container;
			continue;
		}

		/* If we hit this point then we've failed to find an allocation or 
		   block. Attempt to coalesce all available blocks together. */
		if (attempt < MAXIMUM_ALLOC_ATTEMPTS) {
			heap_coalesce(heap);
		}
	}

	/* We failed to find anything */
	klogc(swarn, "*** Failed to allocate on heap: %p\n", heap);
	return NULL;
}

void heap_free(struct heap *heap, void *fptr)
{
	/* We need to find the block in the heap associated with the provided 
	   pointer. */
	struct heap_container *ptr = heap->first_container;
	do {

		for (int i = 0; i < heap->container_blocks; ++i) {
			struct heap_block *blk = &ptr->blocks[i];

			if (blk->offset == (uintptr_t)fptr) {
				/* Remove the allocation mark for this block. */
				blk->flags &= ~heap_block_allocated;
				heap_coalesce(heap);
				return;
			}
		}

	}
	while ((ptr = ptr->next_link) != NULL);

	klogc(swarn, "*** Failed to free '%p' in heap '%p'.\n", fptr, heap);
}