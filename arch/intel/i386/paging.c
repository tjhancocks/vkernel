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

#define PAGE_TABLE_TABLE	0x2		/* Index 2 */
#define PAGE_DIR_TABLE		0x3		/* Index 3 */

struct paging_context __kernel_paging_ctx = { 0 };
paging_info_t kernel_paging_ctx = &__kernel_paging_ctx;

////////////////////////////////////////////////////////////////////////////////

bool page_is_mapped(paging_info_t info, uintptr_t linear);
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
		if (page_is_mapped(kernel_paging_ctx, get_cr2())) {
			panic(
				"Page is mapped, but not present?",
				"An attempt to access page (%p) which was not mapped into the\n"
				"linear address space. The exception occured at EIP: %p\n",
				get_cr2(), frame->eip
			);
		}
		else {
			panic(
				"Page Not Present",
				"An attempt to access page (%p) which was not mapped into the\n"
				"linear address space. The exception occured at EIP: %p\n",
				get_cr2(), frame->eip
			);
		}
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
		paging_map(info, addr, addr);
	}
}

////////////////////////////////////////////////////////////////////////////////

static inline void paging_tlb_invalidate(bool full_shootdown, uintptr_t linear)
{
	if (!paging_is_enabled())
		return;
	
	if (full_shootdown) {
		set_cr3(get_cr3());
	}

	__asm__ volatile("invlpg %0" :: "m"((void *)linear));
}

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

static uintptr_t paging_address_for_table(paging_info_t info, uint32_t table)
{
	struct paging_context *ctx = (void *)info;
	return paging_is_enabled()
		? (uintptr_t)(PAGE_TABLE_TABLE << 22) + (table << 12)
		: (uintptr_t)(
			((union page_table *)ctx->page_dir_physical)[table].s.frame << 12
		  );
}

static uintptr_t paging_address_for_directory(paging_info_t info)
{
	struct paging_context *ctx = (void *)info;
	return paging_is_enabled()
		? paging_address_for_table(info, PAGE_TABLE_TABLE)
		: ctx->page_dir_physical;
}

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
	union page_table *dir = (void *)paging_address_for_directory(info);

	/* Check if the page table exists in the page directory. */
	if (!dir[pd].s.present) {
		klogc(
			sinfo, "Page table %d does not exist in page directory %p\n",
			pd, ctx->page_dir_physical
		);
		return e_fail;
	}

	/* Check if the page exists in the page table */
	union page *table = (void *)paging_address_for_table(info, pd);
	if (!table[pt].s.present) {
		// klogc(sinfo, "Page %d does not exist in page table %d.\n", pt, pd);
		return e_fail;
	}

	/* The page exists. */
	if (f) *f = (table[pt].s.frame << 12);
	return e_ok;
}

////////////////////////////////////////////////////////////////////////////////

static oserr paging_create_table(paging_info_t info, uint32_t table)
{
	struct paging_context *ctx = (void *)info;
	union page_table *dir = (void *)paging_address_for_directory(info);

	klogc(sinfo, "Using page directory %p to create table\n", dir);

	if (dir[table].s.present) {
		/* The page is already present. This operation would be potentially
		   fatal to the operation of the system. */
		klogc(
			swarn, 
			"Attempted to recreate page table %d in page directory %p.\n",
			table, dir
		);
		return e_ok;
	}
	klogc(
		sinfo, "Create Page Table %d in directory %p\n", table, dir
	);

	/* Acquire a new frame for use in the table. */
	uintptr_t table_frame;
	if (table == PAGE_TABLE_TABLE && !paging_is_enabled()) {
		/* This is a special scenario. */
		table_frame = ctx->page_dir_physical;
	}
	else {
		table_frame = pmm_acquire_frame();
	}
	dir[table].s.present = 1;
	dir[table].s.write = 1;
	dir[table].s.frame = table_frame >> 12;

	klogc(
		sok, "Setup entry %d using frame %p, in page directory: %08X (%p)\n", 
		table, table_frame, dir[table].i, dir
	);

	/* Ensure the required page tables exist. */
	if (!dir[PAGE_TABLE_TABLE].s.present) {
		klogc(swarn, "Page table for tracking page tables needs creating.\n");
		/* We do not yet have the page table for tracking page tables. We must
		   get it set up in order to proceed. */
		if (paging_create_table(info, PAGE_TABLE_TABLE) != e_ok) {
			panic(
				"Fatal Error",
				"Failed to setup a page table (%d) to host page tables. "
				"This is considered an unrecoverable error, and the system has "
				"been halted.",
				PAGE_TABLE_TABLE
			);
		}
	}

	/* We're going to map the page table into the appropriate location of the 
	   virtual address space. */
	union page *page_table = (void *)paging_address_for_table(
		info, PAGE_TABLE_TABLE
	);

	klogc(sinfo, "Page table %d has physical address %p\n", table, table_frame);
	klogc(
		sinfo, "Page table %d has linear address %p\n", 
		table, (PAGE_TABLE_TABLE << 22) + (table << 12)
	);
	klogc(sinfo, "Recording to page_table: %p\n", page_table);

	page_table[table].s.present = 1;
	page_table[table].s.write = 1;
	page_table[table].s.frame = table_frame >> 12;

	paging_tlb_invalidate(true, (uintptr_t)page_table);

	/* At this point we know everything succeed correctly. */
	klogc(sok, "Created page table %d successfully.\n", table);
	return e_ok;
}

////////////////////////////////////////////////////////////////////////////////

oserr paging_map(paging_info_t info, uintptr_t frame, uintptr_t linear) 
{
	/* Locate the page table, enter the new page into it and ensure anything
	   required along the way is constructed. If the page is already mapped
	   then warn the user and ignore. */

	bool tlb_shootdown_required = false;
	struct paging_context *ctx = (void *)info;
	union page_table *dir = (void *)paging_address_for_directory(info);

	linear &= ~(PAGE_SIZE - 1);

	if (page_is_mapped(info, linear)) {
		klogc(swarn, "Page %p already exists in directory.\n", linear);
		return e_ok;
	}

	klogc(sinfo, "Mapping page %p to frame %p\n", linear, frame);

	uint32_t pd, pt;
	paging_translate_linear(linear, &pd, &pt);

	/* Ensure the page table exists */
	if (!dir[pd].s.present) {
		/* The page table does not exist. Create it. */
		if (paging_create_table(info, pd) != e_ok) {
			/* Failed to create page table. */
			klogc(swarn, "Failed to create page table %d\n", pd);
			return e_fail;
		}
	}

	/* Locate the page table in memory so we can update it */
	union page *page_table = (void *)paging_address_for_table(info, pd);
	klogc(sinfo, "Mapping uses page table at %p\n", page_table);

	/* Write in the new entry. */
	page_table[pt].s.present = 1;
	page_table[pt].s.write = 1;
	page_table[pt].s.frame = frame >> 12;

	/* Make sure the TLB is flushed if required. */
	paging_tlb_invalidate(false, linear);
	klogc(sok, "Page %p mapped successfully.\n", linear);

	return e_ok;
}

oserr paging_unmap(paging_info_t info, uintptr_t linear)
{
	/* If the address is not actually mapped into the page tables then ignore */
	if (!page_is_mapped(info, linear)) {
		return e_ok;
	}

	uint32_t pd, pt;
	paging_translate_linear(linear, &pd, &pt);

	/* Look up the entry and mark it as not present. Also get the frame and
	   inform the physical memory manager that it is no longer in use. */
	struct paging_context *ctx = info;
	union page *page_table = (void *)paging_address_for_table(info, pd);

	uintptr_t frame = (page_table[pt].s.frame << 12);
	page_table[pt].s.present = 0;
	pmm_release_frame(frame);

	return e_fail;
}

////////////////////////////////////////////////////////////////////////////////

uintptr_t paging_find_linear(paging_info_t info)
{
	/* Work through the page directory and find the first available page that
	   is not used. */
	struct paging_context *ctx = info;
	union page_table *dir = (void *)paging_address_for_directory(info);

	for (uint32_t page = 0; page < (1022 * 1024); ++page) {
		uint32_t pd = page >> 10;
		uint32_t pt = page & 0x3FF;

		if (dir[pd].s.present) {
			union page *table = (void *)paging_address_for_table(info, pd);
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

////////////////////////////////////////////////////////////////////////////////

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
	__kernel_paging_ctx.page_dir = (void *)(
		(PAGE_TABLE_TABLE << 22) | (PAGE_TABLE_TABLE << 12)
	);

	klogc(sinfo, "Kernel Paging Context is %p\n", kernel_paging_ctx);
	klogc(sinfo, "Page dir is located at %p\n", __kernel_paging_ctx.page_dir);
	klogc(
		sinfo, "Page dir frame is located at %p\n", 
		__kernel_paging_ctx.page_dir_physical
	);

	klogc(
		sinfo, "Page dir reported as %p\n", 
		paging_address_for_directory(kernel_paging_ctx)
	);

	/* Create the initial page tables */
	paging_create_table(kernel_paging_ctx, PAGE_TABLE_TABLE);
	paging_create_table(kernel_paging_ctx, PAGE_DIR_TABLE);

	/* Perform any initial identity mapping that is required for the Kernel to
	   operate correctly once paging is enabled. */
	idmap(kernel_paging_ctx, frame_bios);
	idmap(kernel_paging_ctx, frame_kernel_wired);

	/* Install an interrupt handler for the page fault exception. */
	set_int_handler(0x0E, page_fault_handler);

	/* Enable paging */
	paging_set_context(kernel_paging_ctx);
	paging_set_enabled(true);

	return e_ok;
}

#endif