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

#if !defined(CONTEXT_H)
#define CONTEXT_H

#include <types.h>

/**
 The context structure is used to track and associate data to a given "task".
 */
struct context
{
	/* The paging_context references an architecture specific paging context. */
	void *paging_context;

	/* The heap is a context specific heap instance. */
	void *heap;

	/* The stack associated with the given context. */
	void *stack;

	/* The size of the stack in bytes. */
	uint32_t stack_size;
};

extern struct context *kernel_context;

/**
 Construct a new context.
 */
oserr init_context(struct context **ctx);

/**
 Returns the current context.
 */
struct context *current_context(void);

#endif