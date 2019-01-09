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

#if !defined(FORMAT_H)
#define FORMAT_H

#include <types.h>
#include <arch.h>
#include <serial.h>
#include <display.h>
#include <vargs.h>

/**
 The default size of a buffer for the rendering/parsing of format strings is
 just shy of a full page. This is to ensure that the `format_info` structure
 is exactly a page in size.
 */
#define BUFFER_SIZE   (\
	PAGE_SIZE -\
	sizeof(uintptr_t) - sizeof(uintptr_t)\
)

/**
 The format string parser may need to make use of temporary buffers whilst 
 rendering numbers. We do not want to use a full buffer for this. Instead use
 a buffer that is 1/8th of a page.
 */
#define INTERNAL_BUFFER_SIZE	(PAGE_SIZE >> 4)

/**
 The `format_info` structure contains information about a given format type. 
 There are two main format types in _vkernel_, `serial_out` and `display_out`.

 These contain pointers to functions that can be used to write text out via the
 appropriate drivers, or to read text in via the appropriate drivers.

 Please consult the documentation of the appropriate `format_info` instance for
 information regarding their functionalities. 
 */
struct format_info
{
	/**
	 The output buffer for the rendered format string.
	 */
	char buffer[BUFFER_SIZE];

	/**
	 The output function that will be used to write out the rendered format 
	 string. If this NULL, then the rendered format string will be ignored.
	 */
	void(*out)(const char *restrict);

	/**
	 The input function that will be used to get a string to parse.
	 */
	uint32_t(*in)(char *restrict, uint32_t);
} __attribute__((packed));

/**
 Write the given format string to the specified output, after rendering the 
 format tokens with the provided arguments.

 - Note: This function works similarly to the fprintf(...) function.
 */
void write_format(struct format_info *info, const char *restrict fmt,...);

/**
 Write the given format string to the specified output, after rendering the
 format tokens with the provided argument list.

 - Note: This function works similarly to the fprintf(...) function.
 */
void write_format_va(
	struct format_info *info, 
	const char *restrict fmt, 
	va_list va
);


#endif