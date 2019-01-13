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

#include <vmm.h>
#include <pmm.h>
#include <paging.h>
#include <print.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////

static inline void *__vmm_current_context(void)
{
	/* TODO: Identify the current paging context */
	return kernel_paging_ctx;
}

////////////////////////////////////////////////////////////////////////////////

oserr init_virtual_memory(void)
{
	/* Make sure paging is initialised. This is really the only requirement
	   of the virtual memory manager. */
	if (init_paging() != e_ok) {
		klogc(
			serr, "Virtual memory manager was unable to get paging started.\n"
		);
		return e_fail;
	}

	if (vmm_acquire_page(0x01000000) == e_fail) {
		klogc(serr, "Failed to acquire 0x01000000 (1)\n");
	}
	if (vmm_acquire_page(0x01000000) == e_fail) {
		klogc(serr, "Failed to acquire 0x01000000 (2)\n");
	}

	return e_ok;
}

////////////////////////////////////////////////////////////////////////////////

bool vmm_address_valid(uintptr_t address)
{
	/* Ask the Page Manager about the address. This will check it against the
	   current paging context. */
	return page_is_mapped(__vmm_current_context(), address);
}

////////////////////////////////////////////////////////////////////////////////

uintptr_t vmm_acquire_any_page(void)
{
	/* Find the first available linear address in the current context. */
	uintptr_t linear = paging_find_linear(__vmm_current_context());

	/* Attempt to acquire the page. If it fails then return NULL. */
	if (vmm_acquire_page(linear) != e_ok) {
		return 0;
	}

	/* Page acquired successfully. Return its linear address. */
	return linear;
}

oserr vmm_acquire_page(uintptr_t linear)
{
	/* Acquire a physical frame from the PMM and map it to the above linear
	   address. */
	if (!vmm_address_valid(linear)) {
		void *ctx = __vmm_current_context();
		uintptr_t frame = pmm_acquire_frame();
		klogc(sinfo, "vmm_acquire_page(%p) = %p\n", linear, frame);
		if (paging_map(ctx, frame, linear) != e_ok){
			klogc(serr, "Failed to acquire page %p\n", linear);
			return e_fail;
		}

		/* Clear the contents of the page */
		memset((void *)linear, 0, PAGE_SIZE);
	}
	
	/* Acquired the page successfully */
	return e_ok;
}

void vmm_release_page(uintptr_t linear)
{
	/* TODO */
}