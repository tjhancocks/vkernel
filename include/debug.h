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

#if !defined(DEBUG_H)
#define DEBUG_H

#include <types.h>

/**
 Dump the contents of memory over the serial output if it is available. This is
 a useful debugging technique.

 - Warning: This call can be expensive, particularly when dumping large regions
 of memory. Try to avoid leaving calls inside the kernel.

 - ptr 		Reference to the region of memory to dump
 - size		The number of bytes in the region of memory to dump
 - grouping	How many bytes to group together in each chunk. Must be greater than
 			zero.

 An example of a memory dump is:
 0xDEADBEEF | 44 65 61 64 20 42 65 65 66 00 00 00 00 00 00 00 | Dead Beef.......
 */
void memdump(const uint8_t *restrict ptr, uint32_t size, uint32_t grouping);

#endif