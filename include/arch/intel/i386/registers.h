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

#if !defined(REGISTERS_H) && __i386__
#define REGISTERS_H

static inline uint32_t get_cr0(void)
{
	uint32_t cr0;
	__asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
	return cr0;
}

static inline uintptr_t get_cr3(void)
{
	uintptr_t cr3;
	__asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
	return (cr3 & ~(PAGE_SIZE - 1));
}

static inline uintptr_t get_cr2(void)
{
	uintptr_t cr2;
	__asm__ volatile("mov %%cr2, %0" : "=r"(cr2));
	return cr2;
}

static inline void set_cr0(uintptr_t cr0)
{
	__asm__ volatile("mov %0, %%cr0" :: "r"(cr0));
}

static inline void set_cr3(uintptr_t cr3)
{
	__asm__ volatile("mov %0, %%cr3" :: "r"(cr3));
}


#endif