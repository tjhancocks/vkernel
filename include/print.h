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

#if !defined(PRINT_H)
#define PRINT_H

#include <vargs.h>
#include <types.h>

/* Status formatting codes for kprintc functions */
enum print_status { snone, serr, swarn, sok, sinfo };

/**
 Kernel version of a printf() function. This will output via the current 
 processes standard out (typically the system console).

 There are two variants of this function. A plain variant which will output 
 standard text, and a second variant which will output "status" formatted text.
 */
void kprint(const char *restrict fmt,...);
void kprintc(enum print_status status, const char *restrict fmt,...);

/**
 Alternative version of a printf function() that outputs via the current serial
 port configuration. This should be used primarily for debugging and recording
 information that does not need to be kept on the system.

 There are two variants of this function. A plain variant which will output 
 standard text, and a second variant which will output "status" formatted text.
 */
void klog(const char *restrict fmt,...);
void klogc(enum print_status status, const char *restrict fmt,...);

#endif