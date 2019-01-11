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

struct heap;
struct heap_block;

/**
 Possible states that a heap block can be in.
 */
enum heap_state
{
	heap_block_free = 0xF1EEF1EE,
	heap_block_used = 0xA110CA1E,
};

/**
 Block header structure. Contains information about a block of memory on the 
 heap.
 */
struct heap_block
{
	uint32_t state;
	uint32_t size;
	struct heap *owner;
	struct heap_block *next;
	struct heap_block *back;
} __attribute__((packed));

/**
 Heap structure. Contains information about the entire heap.
 */
struct heap 
{
	uintptr_t base;
	uintptr_t limit;
	uint32_t block_count;
	uint32_t free_blocks;
	struct heap_block *first;
	struct heap_block *last;
};

/**
 Initialise a heap with the specified virtual address range.
 */
oserr init_heap(struct heap **heap, uintptr_t base, uintptr_t limit);

/**
 Make an allocation on the heap. Allocates a new block on the heap of the 
 specified size.
 */
void *heap_alloc(struct heap *heap, uint32_t size);

#endif