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

#include <context.h>
#include <arch.h>
#include <heap.h>
#include <print.h>

////////////////////////////////////////////////////////////////////////////////

extern const void *kernel_stack;
struct context *kernel_context = NULL;
static struct context *__current_context = NULL;

////////////////////////////////////////////////////////////////////////////////

/* This entire struct must be no larger than a single page. Perform a static
   assertion to produce a compile time error if this is not the case. */
struct context_header_page{
	struct context ctx;
	struct heap ctx_heap;
};

_Static_assert(
	sizeof(struct context_header_page) <= PAGE_SIZE,
	"context_header_page must be no large than a page in size."
);

////////////////////////////////////////////////////////////////////////////////

static inline void *__ctx_acquire_page(
	void *paging_ctx, uintptr_t base, uintptr_t limit
) {
	uintptr_t addr = find_linear_address(paging_ctx, base, limit, 1);

	if (addr == INVALID_LINEAR_ADDRESS_ERROR) {
		klogc(swarn, "*** Failed to acquire page for context.\n");
		return NULL;
	}
	
	return zero_page(paging_ctx, alloc_page(paging_ctx, (void *)addr, NULL));
}

////////////////////////////////////////////////////////////////////////////////

struct context *current_context(void)
{
	return __current_context;
}

////////////////////////////////////////////////////////////////////////////////

oserr init_context(struct context **ctx)
{
	/* We need to produce a new paging context. Fetch the kernel paging context
	   for now as we'll need to clone it and work from there. */
	struct paging_context *paging_ctx = NULL;
	uintptr_t heap_base = 0x10000000;
	uintptr_t heap_limit = 0xFFFFFFFF;
	if (*ctx == kernel_context) {
		/* We're setting up the kernel context. For this we do not need to
		   clone its paging context. We also need to correct the heap base
		   so that it operates within "kernel" memory. */
		paging_ctx = &kernel_pd;
		heap_base = 0x00400000;
		heap_limit = 0x0FFFFFFF;
		klog("Using existing Kernel Page Context\n");
	}
	else {
		if (clone_kernel_paging_context(&paging_ctx) != e_ok) {
			klogc(
				swarn, "*** Failed to clone the kernel paging context. "
				"Unable to continue initialising context.\n"
			);
			return e_fail;
		}
		klog("Cloned kernel page context\n");
	}

	/* The first job is to construct a header frame for the context. We need 
	   to acquire this page directly as we potentially do not have a heap at
	   this point. Ensure that this allocation is within the kernel! */
	struct context_header_page *header = __ctx_acquire_page(
		paging_ctx, 0x00400000, 0x0FFFFFFF
	);

	/* Ensure the context is correctly mapped and configured. */
	*ctx = header;
	(*ctx)->paging_context = paging_ctx;
	(*ctx)->heap = &header->ctx_heap;

	/* Initialise the heap for the context */
	/* TODO: This is currently all preallocated and thus not overly large 
	   (16MiB). The heap should cover all of the available linear address space
	   and allocate and free pages as required. */
	init_heap(&header->ctx_heap, paging_ctx, 16385);

	/* Setup a stack for the context. If this is the Kernel context then we
	   simply need to adopt the current stack. A stack by default should be
	   16KiB */
	(*ctx)->stack_size = 16385;
	if (*ctx == kernel_context) {
		(*ctx)->stack = &kernel_stack;
	}
	else {
		(*ctx)->stack = heap_alloc((*ctx)->heap, (*ctx)->stack_size);
	}

	/* The context should now be established and ready for use. The last thing
	   to do is ensure there is an active context. Make this context the active
	   one if there isn't already one. */
	if (!__current_context) {
		__current_context = *ctx;
	}
	return e_ok;
}