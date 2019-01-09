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

#if (__i386__ || __x86_64__)

#include <serial.h>
#include <arch/intel/intel.h>
#include <arch.h>

static cpu_port_t __com_serial_ports[] = { 0x3f8, 0x2f8, 0x3e8, 0x2e8 };

struct {
	cpu_port_t port;
	uint8_t com;
} __serial_info;

struct format_info _serial_out = {
	.out = __puts_serial,
};
struct format_info *serial_out = &_serial_out;

void __init_serial(uint8_t com)
{
	if (com <= 0 || com > 4) {
		/* This is an invalid COM port and should be ignored. Leave serial in
		   a disabled state. */
		_serial_out.out = NULL;
		return;
	}
	__serial_info.port = __com_serial_ports[com - 1];
	__serial_info.com = com;

	outb(__serial_info.port + 1, 0x00); /* Disable all interrupts */
	outb(__serial_info.port + 3, 0x80); /* Enable DLAB (set baud rate divisor) */
	outb(__serial_info.port + 0, 0x03); /* Set divisor to 3 (lo) 38400 baud */
	outb(__serial_info.port + 1, 0x00); /*                  (hi) */
	outb(__serial_info.port + 3, 0x03); /* 8 bits, no parity, one stop bit */
	outb(__serial_info.port + 2, 0xc7); /* Enable FIFO, clear them, 14-byte */
	outb(__serial_info.port + 4, 0x0b); /* IRs enabled, RTS/DSR enabled */
}

static inline int __chk_disabled(void)
{
	return (__serial_info.com == 0);
}

static int __chk_write_ready(void)
{
	return inb(__serial_info.port + 5) & 0x20;
}

void __putc_serial(char c)
{
	if (__chk_disabled()) return;
	while (__chk_write_ready() == 0);
	outb(__serial_info.port, c);
}

void __puts_serial(const char *restrict s)
{
	if (__chk_disabled()) return;
	while (*s)
		__putc_serial(*s++);
}

static int __chk_read_ready(void)
{
	return inb(__serial_info.port + 5) & 0x01;
}

char __getc_serial(void)
{
	if (__chk_disabled()) return '\0';
	while (__chk_read_ready() == 0);
	return inb(__serial_info.port);
}

uint32_t __gets_serial(char *restrict s, uint32_t sz)
{
	if (__chk_disabled()) return 0;

	uint32_t len = 0;
	char c = 0;
	
	while ((c = __getc_serial()) && len < sz) {
		if (c == '\n') break;
		s[len++] = c;
	}

	return len;
}

#endif