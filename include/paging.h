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

#if !defined(PAGING_H)
#define PAGING_H

#include <types.h>

/**
 A generic type definition for an architecture specific paging context. The
 paging context contains information about the current paging configuration,
 and layout.
 */
typedef void * paging_info_t;
extern paging_info_t kernel_paging_ctx;

/**
 Check if the system supports a paging environment.
 */
bool paging_is_supported(void);

/**
 Check if paging is currently enabled.
 */
bool paging_is_enabled(void);

/**
 Check if the specified page is mapped or not within the specified paging
 context.
 */
bool page_is_mapped(paging_info_t info, uintptr_t linear);

/**
 Initialise paging in the system.
 */
oserr init_paging(void);

/**
 Map the specified physical frame to the specified linear address.
 */
oserr paging_map(paging_info_t info, uintptr_t frame, uintptr_t linear);

/**
 Unmap the physical memory from the specified linear memory address.
 */
oserr paging_unmap(paging_info_t info, uintptr_t linear);

/**
 Switch to the specified paging context.
 */
oserr paging_set_context(paging_info_t info);

/**
 Translate the specified physical frame into a linear address for the specified
 paging context.
 */
oserr paging_phys_to_linear(paging_info_t info, uintptr_t frame, uintptr_t *a);

/**
 Translate the specified linear address into a physical frame for the specified
 paging context.
 */
oserr paging_linear_to_phys(paging_info_t info, uintptr_t linear, uintptr_t *f);

/**
 Enable/Disable paging functionality on the system.
 */
oserr paging_set_enabled(int flag);

/**
 Determine the first free to use linear address for the specified paging 
 context.
 */
uintptr_t paging_find_linear(paging_info_t info);

/**
 Make sure any paging structures and caches are correctly flushed out to the CPU
 and updated.
 */
void paging_flush(void);

#endif