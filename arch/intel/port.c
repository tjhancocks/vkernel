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

#include <arch/intel/intel.h>

extern void outb(cpu_port_t port, uint8_t value)
{
	__asm__ volatile (
		  "outb %1, %0"
		:
		: "dN"(port), "a"(value)
	);
}

extern uint8_t inb(cpu_port_t port)
{
	uint8_t result = 0;
	__asm__ volatile (
		  "inb %1, %0"
		: "=a"(result)
		: "dN"(port)
	);
	return result;
}

extern void outw(cpu_port_t port, uint16_t value)
{
	__asm__ volatile (
		  "outw %1, %0"
		:
		: "dN"(port), "a"(value)
	);
}

extern uint16_t inw(cpu_port_t port)
{
	uint16_t result = 0;
	__asm__ volatile (
		  "inw %1, %0"
		: "=a"(result)
		: "dN"(port)
	);
	return result;
}

extern void outl(cpu_port_t port, uint32_t value)
{
	__asm__ volatile (
		  "outl %1, %0"
		:
		: "dN"(port), "a"(value)
	);
}

extern uint32_t inl(cpu_port_t port)
{
	uint32_t result = 0;
	__asm__ volatile (
		  "inl %1, %0"
		: "=a"(result)
		: "dN"(port)
	);
	return result;
}

#endif