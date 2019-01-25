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

#if !defined(STRING_H)
#define STRING_H

#include <types.h>

#if !defined(INC_LIBK) || !defined(INC_LIBC)
uint32_t strlen(const char *restrict str);
int strcmp(const char *restrict s0, const char *restrict s1);

void *memset(void *restrict dst, uint8_t v, uint32_t sz);
void *memcpy(void *restrict dst, const void *restrict src, uint32_t n);
#endif

uint32_t ulltoa_base(char *ptr, unsigned long long v, uint8_t base);
uint32_t lltoa_base(char *ptr, signed long long v, uint8_t base);
uint32_t ftoa(char *ptr, double v, int precision);

int atoi(const char *restrict str);

#endif