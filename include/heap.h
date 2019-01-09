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

#if !defined(HEAP_H)
#define HEAP_H

#include <types.h>
#include <arch.h>

enum heap_flags
{
	heap_block_unused = 0,
	heap_block_present = 1 << 0,
	heap_block_allocated = 1 << 31,
};

struct heap_block
{
	uint32_t flags;
	uintptr_t offset;
	uint32_t size;
} __attribute__((packed));

struct heap_container
{
	struct heap_container *next_link;
	struct heap_container *prev_link;
	struct heap_block *blocks;
} __attribute__((packed));

struct heap
{
	uintptr_t page_ctx;
	uintptr_t base;
	uintptr_t limit;
	uint32_t pages;
	uint32_t container_blocks;
	struct heap_container *first_container;
	struct heap_container *last_container;
} __attribute__((packed));

extern struct heap kernel_heap;

void init_heap(
	struct heap *heap, struct paging_context *ctx, uint32_t size_kib
);
void describe_heap(struct heap *heap);
void *heap_alloc(struct heap *heap, uint32_t size);
void heap_free(struct heap *heap, void *ptr);

#endif