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

#if !defined(FRAME_H)
#define FRAME_H

#include <arch.h>
#include <types.h>
#include <multiboot.h>

uint32_t phys_total_kib(void);
uintptr_t phys_kernel_start(void);
uintptr_t phys_kernel_end(void);
uint32_t phys_kernel_frames(void);
uintptr_t phys_modules_start(void);
uintptr_t phys_modules_end(void);
uint32_t phys_modules_frames(void);
uintptr_t phys_frame_stack(void);
uintptr_t phys_frame_stack_ptr(void);
uintptr_t phys_frame_stack_end(void);
uint32_t phys_frame_stack_max(void);
uint32_t phys_frame_stack_free(void);

void *phys_zero_frame(uintptr_t frame);
void *phys_copy_frame(uintptr_t dst, uintptr_t src);

uint32_t phys_frame_count(void);
uintptr_t phys_alloc_frame(void);

void phys_parse_mmap(struct multiboot_info *mb);
void init_phys(struct multiboot_info *mb);

#endif