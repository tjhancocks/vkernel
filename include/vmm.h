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

#if !defined(VIRTUAL_MEMORY_MANAGER_H)
#define VIRTUAL_MEMORY_MANAGER_H

#include <types.h>

/**
 Initialise the virtual memory manager.
 */
oserr init_virtual_memory(void);

/**
 Is the specified address valid? If the address is valid then we can infer the
 address is mapped into the paging structures of the system.
 */
bool vmm_address_valid(uintptr_t address);

/**
 Acquire a new page from the virtual memory manager
 */
uintptr_t vmm_acquire_any_page(void);

/**
 Acquire a specific page from the virtual memory manager.
 */
oserr vmm_acquire_page(uintptr_t linear);

/**
 Release the specified page, returning the frame back to the phyiscal memory
 manager, and marking the page as not present.
 */
void vmm_release_page(uintptr_t linear);

#endif