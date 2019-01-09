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

#include <debug.h>
#include <print.h>

void memdump(const uint8_t *restrict ptr, uint32_t size, uint32_t grouping)
{
	klog(">> MEMORY DUMP %p, %d bytes\n", ptr, size);

	uint32_t rows = size >> 4; /* 16 bytes per row */
	for (uint32_t r = 0; r < rows; ++r) {
		uintptr_t offset = (r << 4);
		klog("%p |", (uintptr_t)ptr + offset);

		for (uint32_t c = 0; c < 16; ++c) {
			if (c == 0 || c % grouping == 0) {
				klog(" %02x", ptr[offset + c]);
			}
			else {
				klog("%02x", ptr[offset + c]);
			}
		}

		klog(" | ");

		for (uint32_t c = 0; c < 16; ++c) {
			char cc = ptr[offset + c];
			klog("%c", cc < 0x20 ? '.' : cc);
		}

		klog("\n");
	}
}