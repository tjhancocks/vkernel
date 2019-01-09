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

#include <types.h>
#include <arch/intel/intel.h>
#include <arch.h>
#include <print.h>
#include <frame.h>
#include <debug.h>
#include <panic.h>

struct paging_context kernel_pd = {0};

////////////////////////////////////////////////////////////////////////////////

static inline void page_indexes(
	uintptr_t addr, 
	uint32_t *p, 
	uint32_t *pd, 
	uint32_t *pt
) {
	uintptr_t aligned_addr = addr & ~(PAGE_SIZE-1);
	*p = PAGE_FOR_ADDR(aligned_addr);
	*pd = (aligned_addr >> 22) & 0x3FF;
	*pt = (aligned_addr >> 12) & 0x3FF;
}

static inline uintptr_t page_for_idx(uint32_t pd, uint32_t pt)
{
	return (pd << 10) | pt;
}

static inline uintptr_t vaddr_for_page(uint32_t page)
{
	return page << 12;
}

static inline uintptr_t vaddr_for_pt(struct paging_context *ctx, uint32_t pd) 
{
	return ctx->linear[pd];
}

static inline void *page_align(void *ptr)
{
	uintptr_t addr = (uintptr_t)ptr;
	return (addr & ~(PAGE_SIZE - 1));
}

static inline void *page_align_next(void *ptr)
{
	uintptr_t addr = (uintptr_t)ptr;
	if (addr & (PAGE_SIZE - 1) == 0) {
		return page_align(ptr);
	}
	else {
		return page_align(ptr) + PAGE_SIZE;
	}
}

////////////////////////////////////////////////////////////////////////////////

void idmap(struct paging_context *ctx, uintptr_t addr, uint32_t flags)
{
	map_page(ctx, addr, addr, flags);
}

void idmap_range(
	struct paging_context *ctx, uintptr_t start, uintptr_t end, uint32_t flags
) {
	for (uintptr_t addr = start; addr < end; addr += PAGE_SIZE) {
		idmap(ctx, addr, flags);
	}
}

////////////////////////////////////////////////////////////////////////////////

uintptr_t find_linear_address(
	struct paging_context *ctx, uintptr_t from, uintptr_t to, uint32_t pages
) {
	/* Ensure the search is possible first of all. */
	uint32_t space = PAGES_FOR_BYTES(to - from);
	if (pages > space) {
		klogc(swarn, "*** pages > space (%d > %d)\n", pages, space);
		return INVALID_LINEAR_ADDRESS_ERROR;
	}

	/* Find the initial page directory entry and work from there. */
	uint32_t page = 0;
	uint32_t pd = 0;
	uint32_t pt = 0;
	page_indexes(from, &page, &pd, &pt);

	/* Find the final page directory entry */
	uint32_t end_page = 0;
	uint32_t end_pd = 0;
	uint32_t end_pt = 0;
	page_indexes(to, &end_page, &end_pd, &end_pt);

	uint32_t count = 0;
	uint32_t start_page = page;
	while (page <= end_page) {
		/* Is the page available? */
		if (ctx->cr3[pd].s.present) {
			/* Are there enough pages in the page table to cover the rest
			   of the required pages? */
			if (ctx->fpages[pd] == 0) {
				count = 0;
				pt = 0;
				++pd;
				page = (page + 0x400) & ~(0x3FF);
				start_page = page;
			}
			else {
				union page *page_table = (void *)ctx->linear[pd];
				if (page_table[pt].s.present) {
					count = 0;
					start_page = page + 1;
				}
				else {
					++count;
				}
				++pt;
				++page;
			}
		}
		else {
			count += 0x400;
			pt += 0x400;
			page += 0x400;
		}

		/* Have we found a good range? If so translate page into an actual
		   linear address and return it. */
		if (count >= pages) {
			return ADDR_FOR_PAGE(start_page);
		}

		/* Increment the search position. */
		if (pt >= 0x400) {
			page += pt;
			pd += (pt / 0x400);
			pt = (pt % 0x400);
		}
	}

	// We failed to find anything.
	return INVALID_LINEAR_ADDRESS_ERROR;
}

////////////////////////////////////////////////////////////////////////////////

void map_page_table(struct paging_context *ctx, uint32_t pdi, uintptr_t frame)
{
	union page_table *pd = (void *)ctx->cr3;
	pd[pdi].s.present = 1;
	pd[pdi].s.write = 1;
	pd[pdi].s.frame = PAGE_FOR_ADDR(frame);

	/* Setup the linear or physical address for the page table, depending on
	   whether paging is enabled or not. If paging is not enabled yet, then
	   the page table _must_ be identity mapped. */
	if (is_paging_enabled()) {
		/* We need to create a linear mapping for the page table. */
		/* TODO: Only search kernel reserved memory! */
		uintptr_t linear = find_linear_address(ctx, 0x0, 0x00800000, 1);
		if (linear == INVALID_LINEAR_ADDRESS_ERROR) {
			panic(
				"Paging Exception",
				"Attempted to create a page table using the frame %p at linear "
				"address %p.",
				frame, linear
			);
		}
		ctx->linear[pdi] = linear;
		map_page(ctx, linear, frame, 0x3);
	}
	else {
		ctx->linear[pdi] = frame;
		idmap(ctx, ctx->linear[pdi], 0x3);
	}
	
}

void map_page(
	struct paging_context *ctx, uintptr_t vaddr, uintptr_t frame, uint32_t flags
) {
	union page_table *pd = (void *)ctx->cr3;

	uint32_t page = 0;
	uint32_t pdi = 0;
	uint32_t pti = 0;
	page_indexes(vaddr, &page, &pdi, &pti);

	/* Check if page table exists */
	if (pd[pdi].s.present == 0) {
		// Page table does not exist
		map_page_table(ctx, pdi, phys_alloc_frame());
	}

	union page *pt = vaddr_for_pt(ctx, pdi);
	pt[pti].i = frame | flags;
}

////////////////////////////////////////////////////////////////////////////////

int page_present(struct paging_context *ctx, void *ptr)
{
	uint32_t page = 0;
	uint32_t pd = 0;
	uint32_t pt = 0;
	page_indexes((uintptr_t)page_align(ptr), &page, &pd, &pt);

	/* Check the page table is present */
	if (!ctx->cr3[pd].s.present) return 0;

	/* Check the page is present */
	union page *page_table = (void *)((uintptr_t *)ctx->linear)[pd];
	return page_table[pt].s.present;
}

uintptr_t phys_for_page(struct paging_context *ctx, void *ptr)
{
	uint32_t page = 0;
	uint32_t pd = 0;
	uint32_t pt = 0;
	page_indexes((uintptr_t)page_align(ptr), &page, &pd, &pt);

	/* Check the page table is present */
	if (!ctx->cr3[pd].s.present) return PAGE_TABLE_NOT_ALLOCATED_ERROR;

	union page *page_table = (void *)((uintptr_t *)ctx->linear)[pd];
	if (!page_table[pt].s.present) return PAGE_NOT_ALLOCATED_ERROR;

	return ADDR_FOR_PAGE(page_table[pt].s.frame);
}

void *copy_page(struct paging_context *ctx, void *dst, void *src)
{
	if (!page_present(ctx, dst)) {
		klogc(swarn, "*** page dst=%p is not present.", page_align(dst));
		return NULL;
	}

	if (!page_present(ctx, src)) {
		klogc(swarn, "*** page src=%p is not present.", page_align(src));
		return NULL;
	}
	
	return memcpy(page_align(dst), page_align(src), PAGE_SIZE);
}

void *zero_page(struct paging_context *ctx, void *dst)
{
	if (!page_present(ctx, dst)) {
		klogc(swarn, "*** page dst=%p is not present.", page_align(dst));
		return NULL;
	}

	return memset(page_align(dst), 0, PAGE_SIZE);
}

void *alloc_page(struct paging_context *ctx, void *ptr, void *frame)
{
	if (page_present(ctx, ptr)) {
		klogc(swarn, "*** page already allocated: %p\n", page_align(ptr));
		return NULL;
	}

	if (!frame)
		frame = (void *)phys_alloc_frame();

	map_page(ctx, (uintptr_t)page_align(ptr), (uintptr_t)frame, 0x3);
	return page_align(ptr);
}

////////////////////////////////////////////////////////////////////////////////

void load_cr3(struct paging_context *ctx)
{
	klogc(sinfo, "Switching CR3 to page directory: %p\n", ctx->cr3);
	set_cr3((uintptr_t)ctx->cr3);
}

void enable_paging(void)
{
	klogc(sok, "Enabling paging now!\n");
	set_cr0(get_cr0() | 0x80000000);
}

void invalidate_tlb(void)
{
	/* TODO */
}

void init_paging(void)
{
	/* We need to setup the initial kernel page directory here, perform any
	   identity mapping that is required, and get the system into a state for
	   paging to be enabled. */
	if (kernel_pd.linear_cr3 || kernel_pd.cr3) {
		klogc(swarn, "*** Attempted to init_paging() a second time.\n");
		return;
	}

	kernel_pd.cr3 = (void *)phys_zero_frame(phys_alloc_frame());
	kernel_pd.linear_cr3 = (uintptr_t)kernel_pd.cr3;
	kernel_pd.linear = (void *)phys_zero_frame(phys_alloc_frame());
	kernel_pd.fpages = (void *)phys_zero_frame(phys_alloc_frame());

	/* The lower 1MiB of memory must be marked present and identity mapped. */
	idmap_range(&kernel_pd, 0x00000000, 0x00100000, 0x3);

	/* The kernel and its modules also need to be memory mapped, as does the  
	   initial page directory and frame stack. */
	idmap_range(&kernel_pd, phys_kernel_start(), phys_modules_end(), 0x3);
	idmap_range(&kernel_pd, phys_frame_stack(), phys_frame_stack_end(), 0x3);
	idmap(&kernel_pd, (uintptr_t)kernel_pd.cr3, 0x3);
	idmap(&kernel_pd, (uintptr_t)kernel_pd.linear, 0x3);
	idmap(&kernel_pd, (uintptr_t)kernel_pd.fpages, 0x3);

	/* We need to perform a final operation, to ensure that fpages 
	   (free page count) is populated with the correct values. This should help
	   finding available page tables in the future much faster. */
	for (uint32_t pd = 0; pd < 0x400; ++pd) {
		if (!kernel_pd.cr3[pd].s.present) {
			kernel_pd.fpages[pd] = 0x400;
		}
		else {
			union page *page_table = (void *)kernel_pd.linear[pd];
			for (uint32_t pt = 0; pt < 0x400; ++pt) {
				kernel_pd.fpages[pd] += (page_table[pt].s.present ? 0 : 1);
			}
		}
	}

	/* Let's enable paging. If everything is setup correctly, then nothing 
	   should appear to change! */
	load_cr3(&kernel_pd);
	enable_paging();
}

oserr clone_kernel_paging_context(struct paging_context **ctx)
{
	/* TODO */
	return e_fail;
}

#endif