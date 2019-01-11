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

#if !defined(SEGMENTS_H) && __i386__
#define SEGMENTS_H

#define KERNEL_CS             0x08
#define KERNEL_DS             0x10
#define TSS_SEG               0x18
#define SELECTOR_TO_IDX(_sel) ((_sel) >> 3)

#define GDT_SIZE               4  /* Zero, Kernel-Code, Kernel-Data, TSS */
#define IDT_SIZE               256
#define IRQ_COUNT              16

#define MAKE_DESCRIPTOR(_base, _lim, _gran, _acc) {\
	.limit_lo = (_lim) & 0xffff,\
	.limit_hi = ((_lim) >> 16) & 0xff,\
	.base_lo = (_base) & 0xffff,\
	.base_mid = ((_base) >> 16) & 0xff,\
	.base_hi = ((_base) >> 24) & 0xff,\
	.granularity = (_gran),\
	.access = (_acc)\
}

struct i386_gate
{
	uint32_t base_lo:16;
	uint32_t selector:16;
	uint32_t zero:8;
	uint32_t access:8;
	uint32_t base_hi:16;
} __attribute__((packed));

struct i386_descriptor
{
	uint32_t limit_lo:16;
	uint32_t base_lo:16;
	uint32_t base_mid:8;
	uint32_t access:8;
	uint32_t limit_hi:4;
	uint32_t granularity:4;
	uint32_t base_hi:8;
} __attribute__((packed));

struct i386_descriptor_pointer
{
	uint16_t size;
	void *ptr;
} __attribute__((packed));

struct i386_interrupt_frame
{
	uint32_t gs;
	uint32_t fs;
	uint32_t es;
	uint32_t ds;
	uint32_t eax;
	uint32_t ecx;
	uint32_t edx;
	uint32_t ebx;
	uint32_t esp;
	uint32_t ebp;
	uint32_t esi;
	uint32_t edi;
	uint32_t interrupt;
	uint32_t errc;
	uint32_t eip;
	uint32_t cs;
	uint32_t eflags;
	uint32_t user_esp;
	uint32_t ss;
} __attribute__((packed));

extern struct i386_descriptor master_gdt[GDT_SIZE];
extern struct i386_tss master_tss;
extern struct i386_gate master_idt[IDT_SIZE];

typedef void(*irq_handler_t)(uint8_t irq);
typedef void(*int_handler_t)(struct i386_interrupt_frame *frame);

void i386_map_gate(uint8_t num, void(*f)(void), uint16_t sel, uint8_t acc);
void set_irq_handler(uint8_t irq, irq_handler_t fn);
void set_int_handler(uint8_t num, int_handler_t fn);

void init_gdt(void);
void init_idt(void);

void ack_master_pic();
void ack_slave_pic();
void init_pic();

#endif