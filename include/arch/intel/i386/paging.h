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

#if !defined(i386_PAGING_H) && __i386__
#define i386_PAGING_H

#define PAGE_SIZE 0x1000

struct paging_context 
{
	union page_table *page_dir;
	uintptr_t page_dir_physical;
} __attribute__((packed));

union page 
{
	struct {
		uint32_t present:1;
		uint32_t write:1;
		uint32_t user:1;
		uint32_t write_through:1;
		uint32_t cache_disable:1;
		uint32_t accessed:1;
		uint32_t dirty:1;
		uint32_t pat:1;
		uint32_t ignored:4;
		uint32_t frame:20;
	} __attribute__((packed)) s;
	uint32_t i;
};

union page_table 
{
	struct {
		uint32_t present:1;
		uint32_t write:1;
		uint32_t user:1;
		uint32_t write_through:1;
		uint32_t cache_disable:1;
		uint32_t accessed:1;
		uint32_t ignored1:1;
		uint32_t large:1;
		uint32_t ignored2:4;
		uint32_t frame:20;
	} __attribute__((packed)) s;
	uint32_t i;
};

extern struct paging_context kernel_pd;

#endif