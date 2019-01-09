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

#include <ramdisk.h>
#include <types.h>
#include <heap.h>
#include <tar.h>
#include <string.h>

struct ramdisk system_ramdisk = { 0 };

void init_ramdisk(struct ramdisk *rd, uintptr_t start, uintptr_t end)
{
	if (rd == NULL)
		return;

	/* Set the basic information about the ramdisk. */
	rd->start = start;
	rd->end = end;

	parse_tar(
		(void *)rd->start, 
		rd->end - rd->start, &rd->file_count, (void **)&rd->files
	);
}

int chk_ramdisk(struct ramdisk *rd)
{
	return (rd && rd->start < rd->end);
}

const void *ramdisk_open(struct ramdisk *rd, const char *name, uint32_t *size)
{
	if (rd == NULL)
		return NULL;

	for (int i = 0; i < rd->file_count; ++i) {
		if (strcmp(name, rd->files[i].name) == 0) {
			/* Found the file. */
			if (size) {
				*size = rd->files[i].size;
			}
			return (void *)rd->files[i].offset;
		}
	}

	return NULL;
}