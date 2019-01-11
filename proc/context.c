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
#include <vmm.h>
#include <paging.h>

////////////////////////////////////////////////////////////////////////////////

extern const void *kernel_stack;
struct context *kernel_context = NULL;
static struct context *__current_context = NULL;

////////////////////////////////////////////////////////////////////////////////

/* This entire struct must be no larger than a single page. Perform a static
   assertion to produce a compile time error if this is not the case. */
struct context_header_page{
	struct context ctx;
	struct heap *ctx_heap;
};

_Static_assert(
	sizeof(struct context_header_page) <= PAGE_SIZE,
	"context_header_page must be no large than a page in size."
);

////////////////////////////////////////////////////////////////////////////////

static inline void *__ctx_acquire_page(void) {
	return (void *)vmm_acquire_any_page();
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
	uintptr_t heap_limit = 0xC0000000;
	if (*ctx == kernel_context) {
		/* We're setting up the kernel context. For this we do not need to
		   clone its paging context. We also need to correct the heap base
		   so that it operates within "kernel" memory. */
		paging_ctx = kernel_paging_ctx;
		heap_base = 0x00400000; /* 4MiB */
		heap_limit = 0x10000000; /* 256MiB */
		klog("Using existing Kernel Page Context: %p\n", kernel_paging_ctx);
	}
	else {
		/*
		if (clone_kernel_paging_context(&paging_ctx) != e_ok) {
			klogc(
				swarn, "*** Failed to clone the kernel paging context. "
				"Unable to continue initialising context.\n"
			);
			return e_fail;
		}
		klog("Cloned kernel page context. Switching to it.\n");
		load_cr3(paging_ctx);
		*/
	}

	/* The first job is to construct a header frame for the context. We need 
	   to acquire this page directly as we potentially do not have a heap at
	   this point. Ensure that this allocation is within the kernel! */
	struct context_header_page *header = __ctx_acquire_page();

	/* Ensure the context is correctly mapped and configured. */
	*ctx = header;
	(*ctx)->paging_context = paging_ctx;

	/* Initialise the heap for the context */
	init_heap(&header->ctx_heap, heap_base, heap_limit);
	(*ctx)->heap = header->ctx_heap;

	/* Setup a stack for the context. If this is the Kernel context then we
	   simply need to adopt the current stack. A stack by default should be
	   16KiB */
	(*ctx)->stack_size = 16385;
	if (*ctx == kernel_context) {
		(*ctx)->stack = &kernel_stack;
	}
	else {
		uint32_t sp = 0;

		(*ctx)->stack = heap_alloc((*ctx)->heap, (*ctx)->stack_size);
		if (init_stack((*ctx)->stack, NULL, &sp) != e_ok) {
			kprintc(serr, "Failed to setup stack for context.\n");
			return e_fail;
		}
		(*ctx)->stack_ptr = (void *)sp;
	}

	/* The context should now be established and ready for use. The last thing
	   to do is ensure there is an active context. Make this context the active
	   one if there isn't already one. */
	if (!__current_context) {
		__current_context = *ctx;
	}
	return e_ok;
}