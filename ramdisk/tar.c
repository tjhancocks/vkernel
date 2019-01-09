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

#include <tar.h>
#include <types.h>
#include <alloc.h>

////////////////////////////////////////////////////////////////////////////////

static inline uint32_t __tar_getsize(struct tar_header *restrict file)
{
	uint32_t size = 0;
	for (int i = 0; i <= 10; ++i) {
		size = (size << 3) | (file->size[i] - '0');
	}
	return size;
}

static inline const char *__tar_filename(const char *restrict tar_path) {
	char *name = kalloc(sizeof(*name) * 100);
	char *n = name;
	char *c = tar_path;

	while (*c) {
		if (*c == '/') {
			n = name;
			++c;
			continue;
		}
		*n++ = *c++;
	}


	*n = '\0';
	return name;
}

////////////////////////////////////////////////////////////////////////////////

void parse_tar(
	const void *restrict tar,
	uint32_t size, 
	uint32_t *file_count,
	struct tar_file **files
) {
	uint32_t count = 0;

	/* Work out how many files there are first of all. */
	struct tar_header *ptr = tar;
	while (*ptr->path != '\0') {
		++count;

		/* Determine how large the file is, so that we can locate the next
		   file header in the tarball. We're not currently concerned with 
		   recording this information as we do not yet have memory to record
		   this within. */
		uint32_t tar_file_size = __tar_getsize(ptr);

		/* Move to the next tar file header */
		uintptr_t addr = (uintptr_t)ptr;
		ptr = (void *)(
			addr + (((tar_file_size >> 9) + 1) << 9) + 
			(tar_file_size % 512 != 0 ? 512 : 0)
		);
	}

	/* Allocate the appropriate memory and then construct a list of files to 
	   pass back to the caller. */
	if (files) {
		*files = kalloc(sizeof(**files) * count);
		
		struct tar_header *ptr = tar;
		for (int i = 0; i < count; ++i) {
			struct tar_file *file = *files;

			/* Extract the appropriate information and record it. */
			file[i].name = __tar_filename(&ptr->path);
			file[i].size = __tar_getsize(ptr);
			file[i].offset = (uintptr_t)ptr + 512;

			/* Move to the next tar file header */
			uintptr_t addr = (uintptr_t)ptr;
			ptr = (void *)(
				addr + (((file[i].size >> 9) + 1) << 9) + 
				(file[i].size % 512 != 0 ? 512 : 0)
			);
		}
	}

	if (file_count)
		*file_count = count;
}
