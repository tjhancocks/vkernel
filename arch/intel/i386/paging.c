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

#if __i386__

#include <paging.h>
#include <arch.h>
#include <pmm.h>
#include <print.h>
#include <string.h>
#include <debug.h>

////////////////////////////////////////////////////////////////////////////////

#define MAX_TMP_PAGES	16

struct paging_context __kernel_paging_ctx = { 0 };
paging_info_t kernel_paging_ctx = &__kernel_paging_ctx;

static struct {
	uint32_t present:1;
	uint32_t in_use:1;
	uint32_t unused:10;
	uintptr_t page:20;
} __attribute__((packed, aligned(4))) __tmp_pages[MAX_TMP_PAGES];

////////////////////////////////////////////////////////////////////////////////

static void page_fault_handler(struct i386_interrupt_frame *frame)
{
	/* Determine what to do with the fault. If nothing is done, then panic. */
	if (frame->errc & 0x01) {
		/* The error is a page-protection violation. */
		panic(
			"Illegal Page Access", 
			"An attempt to access a page without sufficient privellages."
		);
	}
	else {
		/* Page is not present. Should it be created? */
		panic(
			"Page Not Present",
			"An attempt to access page (%p) which was not mapped into the\n"
			"linear address space. The exception occured at EIP: %p\n",
			get_cr2(), frame->eip
		);
	}

	/* A generic catch all in case something is missed. */
	panic("Page Fault Occurred", "An unhandled page fault has occurred.");
}

////////////////////////////////////////////////////////////////////////////////

static inline void idmap(paging_info_t info, enum frame_purpose purpose)
{
	uintptr_t start, end;
	pmm_frame_range(purpose, &start, &end);
	klogc(sinfo, "idmap(%p -> %p)\n", start, end);
	for (uintptr_t addr = start; addr < end; addr += PAGE_SIZE) {
		paging_map(info, addr, addr, 0);
	}
}

oserr init_paging(void)
{
	/* Should we proceed? If paging is not supported, or already enabled then
	   we should not. */
	if (!paging_is_supported()) {
		klogc(serr, "Paging is not supported.\n"); /* Should not happen */
		return e_fail;
	}

	if (paging_is_enabled()) {
		klogc(
			swarn, "Attempted to initialise paging a second time... Ignoring.\n"
		);
		return e_ok;
	}

	/* Acquire any required memory for the kernel paging context to work 
	   correctly. */
	__kernel_paging_ctx.page_dir_physical = pmm_acquire_frame();
	__kernel_paging_ctx.page_dir = (void*)__kernel_paging_ctx.page_dir_physical;
	__kernel_paging_ctx.page_tables_linear = (void *)pmm_acquire_frame();

	klogc(sinfo, "Kernel Paging Context is %p\n", kernel_paging_ctx);
	klogc(sinfo, "Page dir is located at %p\n", __kernel_paging_ctx.page_dir);

	/* Make sure the temporary pages are also established. */
	for (int i = 0; i < MAX_TMP_PAGES; ++i) {
		__tmp_pages[i].page = (pmm_acquire_frame() >> 12);
		__tmp_pages[i].present = 1;
	}

	/* Perform any initial identity mapping that is required for the Kernel to
	   operate correctly once paging is enabled. */
	idmap(kernel_paging_ctx, frame_bios);
	idmap(kernel_paging_ctx, frame_kernel_wired);

	/* Make sure the initial page directory and page table are mapped. */
	paging_map(
		kernel_paging_ctx, 
		__kernel_paging_ctx.page_dir_physical,
		__kernel_paging_ctx.page_dir_physical,
		0
	);

	paging_map(
		kernel_paging_ctx, 
		(uintptr_t)__kernel_paging_ctx.page_tables_linear,
		(uintptr_t)__kernel_paging_ctx.page_tables_linear,
		0
	);

	paging_map(
		kernel_paging_ctx, 
		((uintptr_t *)__kernel_paging_ctx.page_tables_linear)[0],
		((uintptr_t *)__kernel_paging_ctx.page_tables_linear)[0],
		0
	);

	/* Install an interrupt handler for the page fault exception. */
	set_int_handler(0x0E, page_fault_handler);

	/* Enable paging */
	paging_set_context(kernel_paging_ctx);
	paging_set_enabled(true);

	return e_ok;
}

////////////////////////////////////////////////////////////////////////////////

static inline void *__acquire_tmp_page(void)
{
	klogc(sinfo, "Acquring temporary page.\n");
	/* Search through the list of temporary pages for one that is not being
	   used currently. */
	for (int i = 0; i < MAX_TMP_PAGES; ++i) {
		if (!__tmp_pages[i].in_use && __tmp_pages[i].present) {
			uintptr_t address = __tmp_pages[i].page << 12;
			klogc(sinfo, "Marking temporary page %p as in use.\n", address);
			__tmp_pages[i].in_use = 1;
			return (void *)address;
		}
	}
	klogc(swarn, "Failed to acquire a temporary page.\n");
	return NULL;
}

static inline void __release_tmp_page(void *ptr)
{
	/* Search through the list of temporary pages for the specified one, and
	   mark it as available. */
	for (int i = 0; i < MAX_TMP_PAGES; ++i) {
		uintptr_t page = ((uintptr_t)ptr & 0xFFFFF000) >> 12;
		if (__tmp_pages[i].page == page) {
			klogc(sinfo, "Marking temporary page %p as not in use.\n", ptr);
			__tmp_pages[i].in_use = 0;
			return;
		}
	}
	klogc(swarn, "Attempted to use page %p as a temporary one.\n", ptr);
}

////////////////////////////////////////////////////////////////////////////////

bool paging_is_supported(void)
{
	/* TODO: Actually confirm that this is the case? */
	return 1;
}

bool paging_is_enabled(void)
{
	return (get_cr0() & (1 << 31));
}

oserr paging_set_enabled(int flag)
{
	set_cr0(get_cr0() | (1 << 31));
	return e_ok;
}

oserr paging_set_context(paging_info_t info)
{
	if (!info) {
		klogc(swarn, "Attempted to switch to a NULL paging context.\n");
		return e_fail;
	}

	struct paging_context *ctx = info;
	if (ctx->page_dir_physical & 0xFFF) {
		klogc(
			swarn, "Page directory is not correctly aligned! %p\n", 
			ctx->page_dir_physical
		);
		return e_fail;
	}

	/* Set the CR3 register to the page directory for the given context */
	set_cr3(ctx->page_dir_physical);
	klogc(sok, "CR3 set to %p\n", get_cr3());
}

////////////////////////////////////////////////////////////////////////////////

static inline void paging_translate_linear(
	uintptr_t linear, uint32_t *pd, uint32_t *pt
) {
	/* Translate a linear address into page directory and page table indexes. */
	*pd = (linear >> 22) & 0x3FF;
	*pt = (linear >> 12) & 0x3FF;
}

static inline uintptr_t paging_translate_index(uint32_t pd, uint32_t pt)
{
	/* Translate a set of page directory and page table indexes to a linear
	   address. */
	return ((pd & 0x3FF) << 22) | ((pt & 0x3FF) << 12);
}

////////////////////////////////////////////////////////////////////////////////

bool page_is_mapped(paging_info_t info, uintptr_t linear)
{
	return (paging_linear_to_phys(info, linear, NULL) == e_ok);
}

oserr paging_phys_to_linear(paging_info_t info, uintptr_t frame, uintptr_t *a)
{
	/* TODO */
	klogc(swarn, "%s:%s is not implemented.\n", __FILE__, __FUNCTION__);
	return e_fail;
}

oserr paging_linear_to_phys(paging_info_t info, uintptr_t linear, uintptr_t *f)
{
	/* Check if the page exists. If it does not look up the phyiscal frame */

	/* The first task is to determine where in the paging directory the address
	   points to. */
	uint32_t pd, pt;
	paging_translate_linear(linear, &pd, &pt);

	/* Fetch the current page directory and page table for the page context. */
	struct paging_context *ctx = info;

	/* Check if the page table exists in the page directory. */
	if (!ctx->page_dir[pd].s.present) {
		klogc(
			sinfo, "Page table %d does not exist in page directory %p\n",
			pd, ctx->page_dir_physical
		);
		return e_fail;
	}

	/* Check if the page exists in the page table */
	union page *table = (void *)(ctx->page_tables_linear[pd]);
	if (!table[pt].s.present) {
		klogc(sinfo, "Page %d does not exist in page table %d.\n", pt, pd);
		return e_fail;
	}

	/* The page exists. */
	if (f) *f = (table[pt].s.frame << 12);
	return e_ok;
}

////////////////////////////////////////////////////////////////////////////////

static uintptr_t validate_linear_address(paging_info_t info, uintptr_t linear)
{
	/* Make sure the specified linear address is still available for use.
	   This is necessary because of the page table creation strategy being
	   employed. */
	klogc(sinfo, "Validating the availablity of linear address %p\n", linear);

	/* We're going to work repeatedly check, moving the linear address on by
	   a single page each time we find the linear address to be invalid. */
	bool is_valid = true;
	do {
		if (page_is_mapped(info, linear) == false) {
			linear += PAGE_SIZE;
			is_valid = false;
		}
		else {
			is_valid = true;
		}
	} while (!is_valid);

	/* We now have a valid linear address */
	klogc(sok, "Updated linear address to %p\n", linear);
	return linear;
}

oserr paging_map(
	paging_info_t info, uintptr_t frame, uintptr_t linear, uint8_t flags
) {
	/* This is reasonably complex. We have multiple scenarios to consider when
	   mapping a page.

	   1. Paging is not yet enabled.
	   2. Page table exists and there is no current entry.
	   3. Page table exists and there is a current entry.
	   4. Page table does not exist, but there is an available page for it.
	   5. Page table does not exist, and requires a new page table for the page 
	      table to be created.
	   */

	struct paging_context *ctx = info;
	klogc(sinfo, "Mapping frame %p to linear %p\n", frame, linear);

	/* First of all, determine where the new entry needs to exist, and whether
	   there is currently a page table there or not. This is common across all 
	   paths. */
	uint32_t pd, pt;
	paging_translate_linear(linear, &pd, &pt);
	klogc(sinfo, "Linear translation: pd=%d pt=%d\n", pd, pt);

	if (!ctx->page_dir[pd].s.present) {
		klogc(sinfo, "Required page table %d needs to be created\n", pd);

		/* No page table currently exists. We need to create one. We need to
		   careful not to dereference the new frame however, as when paging is
		   enabled, it could cause a #GPF. */
		uintptr_t pt_frame = pmm_acquire_frame();
		uintptr_t pt_linear = 0;
		klogc(
			sinfo, "Using frame %p for page table %d.\n", pt_frame, pd
		);

		if (!paging_is_enabled()) {
			/* Paging is not enabled. We can dereference the frame as needed. */
			memset((void *)pt_frame, 0, PAGE_SIZE);

			klogc(
				sinfo, "Mapping page table %d to linear %p\n", 
				pt_linear, pd
			);

			/* Add the page table into the directory. */
			klogc(sinfo, "directory = %p, pd = %d\n", ctx->page_dir, pd);
			ctx->page_dir[pd].s.frame = (pt_frame >> 12);
			ctx->page_dir[pd].s.present = 1;
			ctx->page_dir[pd].s.write = 1;

			/* Map the page table as a page, using identity mapping. */
			pt_linear = pt_frame;
			if (paging_map(info, pt_frame, pt_linear, flags) != e_ok) {
				klogc(
					serr, "Failed to identity map page table %d (%p)\n",
					pd, pt_linear
				);
				return e_fail;
			}

			/* Save the linear address of the page table for future use. */
			ctx->page_tables_linear[pd] = pt_linear;
		}
		else {
			/* Paging is enabled. We must not dereference the page table frame
			   until we have mapped into memory. This may pose a chicken and the
			   egg style problem. 

			   Map the frame into a temporary page so that we can use it. */
			void *tmp = __acquire_tmp_page();
			if (paging_map(info, pt_frame, (uintptr_t)tmp, flags) != e_ok) {
				klogc(
					serr, 
					"Failed to map page table %d (%p) against tmp page %p\n",
					pd, pt_frame, tmp
				);
				__release_tmp_page(tmp);
				return e_fail;
			}
			memset(tmp, 0, PAGE_SIZE);

			/* Add the page table into the directory. */
			ctx->page_dir[pd].s.frame = (pt_frame >> 12);
			ctx->page_dir[pd].s.present = 1;
			ctx->page_dir[pd].s.write = 1;

			__release_tmp_page(tmp);

			/* Now that the frame is successfully cleared, we need to map the
			   page table into the linear address space so we can use it 
			   properly. */
			pt_linear = paging_find_linear(info);
			if (paging_map(info, pt_frame, pt_linear, flags) != e_ok) {
				klogc(
					serr, "Failed to map page table %d (%p) to linear %p\n",
					pd, pt_frame, pt_linear
				);
				return e_fail;
			}

			pt_linear = validate_linear_address(info, pt_linear);

			/* Save the linear address of the page table for future use. */
			ctx->page_tables_linear[pd] = pt_linear;
		}
	}

	/* Check to ensure there isn't already an entry for the page. */
	union page *table = (void *)(ctx->page_tables_linear[pd]);
	if (table[pt].s.present) {
		klogc(swarn, "Page entry already exists for %p\n", linear);
		if (flags & f_existing) {
			/* Page is already mapped. If it is mapped to a different frame
			   then ensure the new frame is released. */
			if (table[pt].s.frame != (frame >> 12)) {
				pmm_release_frame(frame);
			}
			return e_ok;
		}
		else {
			/* Page is already mapped, and it is an illegal operation to 
			   remap. */
			return e_fail;
		}
	}

	klogc(
		sinfo, "Completing the mapping of frame %p to linear %p\n", 
		frame, linear
	);

	/* At this point the page table should exist. We can now map the page */
	table[pt].s.frame = (frame >> 12);
	table[pt].s.present = 1;
	table[pt].s.write = 1;

	/* All done. */
	return e_ok;
}

////////////////////////////////////////////////////////////////////////////////

uintptr_t paging_find_linear(paging_info_t info)
{
	/* Work through the page directory and find the first available page that
	   is not used. */
	struct paging_context *ctx = info;
	for (uint32_t page = 0; page < (1024 * 1024); ++page) {
		uint32_t pd = page >> 10;
		uint32_t pt = page & 0x3FF;

		if (ctx->page_dir[pd].s.present) {
			union page *table = (void *)(ctx->page_tables_linear[pd]);
			if (!table[pt].s.present) {
				return (pd << 22) | (pt << 12);
			}
		}
		else {
			return (pd << 22) | (pt << 12);
		}
	}
	return 0;
}

#endif