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

#include <print.h>
#include <format.h>

////////////////////////////////////////////////////////////////////////////////

void kprintc_va(enum print_status status, const char *restrict fmt, va_list va)
{
	switch (status) {
		case serr:
			display_set_attribute(0x04);
			write_format(display_out, "[error] ");
			break;
		case swarn:
			display_set_attribute(0x06);
			write_format(display_out, "[warn] ");
			break;
		case sok:
			display_set_attribute(0x02);
			write_format(display_out, "[ ok ] ");
			break;
		case sinfo:
			display_set_attribute(0x03);
			write_format(display_out, "[info] ");
		default: 
			break;
	}

	write_format_va(display_out, fmt, va);
	display_set_attribute(0x07);
}

void kprint(const char *restrict fmt,...)
{
	va_list va;
	va_start(va, fmt);
	kprintc_va(snone, fmt, va);
	va_end(va);
}

void kprintc(enum print_status status, const char *restrict fmt,...)
{
	va_list va;
	va_start(va, fmt);
	kprintc_va(status, fmt, va);
	va_end(va);
}
 
////////////////////////////////////////////////////////////////////////////////

void klogc_va(enum print_status status, const char *restrict fmt, va_list va)
{
	switch (status) {
		case serr: 
			write_format(serial_out,  "[error] ");
			break;
		case swarn: 
			write_format(serial_out, "[warn] ");
			break;
		case sok: 
			write_format(serial_out,   "[ ok ] ");
			break;
		case sinfo:
			write_format(serial_out, "[info] ");
		default: 
			break;
	}

	write_format_va(serial_out, fmt, va);
}

void klog(const char *restrict fmt,...)
{
	va_list va;
	va_start(va, fmt);
	klogc_va(snone, fmt, va);
	va_end(va);
}

void klogc(enum print_status status, const char *restrict fmt,...)
{
	va_list va;
	va_start(va, fmt);
	klogc_va(status, fmt, va);
	va_end(va);
}