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

#if !defined(RAMDISK_H)
#define RAMDISK_H

#include <types.h>

struct ramdisk_entry
{
	const char *name;
	uintptr_t offset;
	uint32_t size;
};

struct ramdisk
{
	uintptr_t start;
	uintptr_t end;
	uint32_t file_count;
	struct ramdisk_entry *files;
} __attribute__((packed));

extern struct ramdisk system_ramdisk;

void init_ramdisk(struct ramdisk *rd, uintptr_t start, uintptr_t end);
int chk_ramdisk(struct ramdisk *rd);

const void *ramdisk_open(struct ramdisk *rd, const char *name, uint32_t *size);

#endif