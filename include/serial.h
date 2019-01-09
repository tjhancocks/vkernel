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

#include <types.h>
#include <format.h>
#include <arch.h>

#if !defined(SERIAL_H)
#define SERIAL_H

extern void __init_serial(uint8_t com);
extern void __putc_serial(char c);
extern void __puts_serial(const char *restrict s);
extern char __getc_serial(void);
extern uint32_t __gets_serial(char *restrict s, uint32_t sz);

extern struct format_info *serial_out;

#if defined(USE_SERIAL)
#	define init_serial(_com)	(__init_serial((_com)))
#	define putc_serial(_c)		(__putc_serial((_c)))
#	define puts_serial(_s)		(__puts_serial((_s)))
#	define getc_serial(_c)		(__getc_serial())
#	define gets_serial(_s, _sz)	(__gets_serial((_s), (_sz)))
#else
#	define init_serial(_com)
#	define putc_serial(_c)
#	define puts_serial(_s)
#	define getc_serial(_c)		'\0'
#	define gets_serial(_s, _sz) (0)
#endif

#endif