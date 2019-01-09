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

#if __i386__

#include <arch/intel/intel.h>
#include <serial.h>

#define MASTER_PIC            0x20
#define SLAVE_PIC             0xA0
#define MASTER_PIC_CMD        MASTER_PIC
#define SLAVE_PIC_CMD         SLAVE_PIC
#define MASTER_PIC_OFF        0x20
#define SLAVE_PIC_OFF         0x28
#define MASTER_PIC_DATA       MASTER_PIC + 0x01
#define SLAVE_PIC_DATA        SLAVE_PIC + 0x01

#define PIC_EOI               0x20

void ack_master_pic()
{
	outb(MASTER_PIC_CMD, PIC_EOI);
}

void ack_slave_pic()
{
	outb(SLAVE_PIC_CMD, PIC_EOI);
}

void init_pic()
{
	outb(MASTER_PIC_CMD, 0x11);
	outb(SLAVE_PIC_CMD, 0x11);
	outb(MASTER_PIC_DATA, MASTER_PIC_OFF);
	outb(SLAVE_PIC_DATA, SLAVE_PIC_OFF);
	outb(MASTER_PIC_DATA, 0x04);
	outb(SLAVE_PIC_DATA, 0x02);
	outb(MASTER_PIC_DATA, 0x01);
	outb(SLAVE_PIC_DATA, 0x01);
	outb(MASTER_PIC_DATA, 0x00);
	outb(SLAVE_PIC_DATA, 0x00);
}

#endif
