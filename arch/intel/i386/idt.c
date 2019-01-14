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
#include <arch.h>
#include <print.h>
#include <panic.h>
#include <thread.h>

extern void _isr0x00(void);
extern void _isr0x01(void);
extern void _isr0x02(void);
extern void _isr0x03(void);
extern void _isr0x04(void);
extern void _isr0x05(void);
extern void _isr0x06(void);
extern void _isr0x07(void);
extern void _isr0x08(void);
extern void _isr0x09(void);
extern void _isr0x0a(void);
extern void _isr0x0b(void);
extern void _isr0x0c(void);
extern void _isr0x0d(void);
extern void _isr0x0e(void);
extern void _isr0x0f(void);
extern void _isr0x10(void);
extern void _isr0x11(void);
extern void _isr0x12(void);
extern void _isr0x13(void);
extern void _isr0x14(void);
extern void _isr0x15(void);
extern void _isr0x16(void);
extern void _isr0x17(void);
extern void _isr0x18(void);
extern void _isr0x19(void);
extern void _isr0x1a(void);
extern void _isr0x1b(void);
extern void _isr0x1c(void);
extern void _isr0x1d(void);
extern void _isr0x1e(void);
extern void _isr0x1f(void);
extern void _irq0x20(void);
extern void _irq0x21(void);
extern void _irq0x22(void);
extern void _irq0x23(void);
extern void _irq0x24(void);
extern void _irq0x25(void);
extern void _irq0x26(void);
extern void _irq0x27(void);
extern void _irq0x28(void);
extern void _irq0x29(void);
extern void _irq0x2a(void);
extern void _irq0x2b(void);
extern void _irq0x2c(void);
extern void _irq0x2d(void);
extern void _irq0x2e(void);
extern void _irq0x2f(void);

struct i386_gate master_idt[IDT_SIZE] __attribute__((aligned(PAGE_SIZE)));
struct i386_descriptor_pointer master_idt_ptr;
static irq_handler_t _irq_handlers[IRQ_COUNT] = { NULL };
static int_handler_t _int_handlers[IDT_SIZE] = { NULL };

static inline void lidt(struct i386_descriptor_pointer *ptr)
{
	__asm__ volatile(
		  "lidt %0"
		:
		: "m"(*ptr)
	);
}

void i386_map_gate(uint8_t num, void(*f)(void), uint16_t sel, uint8_t acc)
{
	uintptr_t base = (uintptr_t)f;
	master_idt[num].base_lo = base & 0xffff;
	master_idt[num].base_hi = (base >> 16) & 0xffff;
	master_idt[num].selector = sel;
	master_idt[num].access = acc | 0x60;
}

void init_idt(void)
{
	/* Map all Interrupt Service Routines (ISRs) into the master IDT. */
	i386_map_gate(0x00, _isr0x00, KERNEL_CS, 0x8E);
	i386_map_gate(0x01, _isr0x01, KERNEL_CS, 0x8E);
	i386_map_gate(0x02, _isr0x02, KERNEL_CS, 0x8E);
	i386_map_gate(0x03, _isr0x03, KERNEL_CS, 0x8E);
	i386_map_gate(0x04, _isr0x04, KERNEL_CS, 0x8E);
	i386_map_gate(0x05, _isr0x05, KERNEL_CS, 0x8E);
	i386_map_gate(0x06, _isr0x06, KERNEL_CS, 0x8E);
	i386_map_gate(0x07, _isr0x07, KERNEL_CS, 0x8E);
	i386_map_gate(0x08, _isr0x08, KERNEL_CS, 0x8E);
	i386_map_gate(0x09, _isr0x09, KERNEL_CS, 0x8E);
	i386_map_gate(0x0a, _isr0x0a, KERNEL_CS, 0x8E);
	i386_map_gate(0x0b, _isr0x0b, KERNEL_CS, 0x8E);
	i386_map_gate(0x0c, _isr0x0c, KERNEL_CS, 0x8E);
	i386_map_gate(0x0d, _isr0x0d, KERNEL_CS, 0x8E);
	i386_map_gate(0x0e, _isr0x0e, KERNEL_CS, 0x8E);
	i386_map_gate(0x0f, _isr0x0f, KERNEL_CS, 0x8E);
	i386_map_gate(0x10, _isr0x10, KERNEL_CS, 0x8E);
	i386_map_gate(0x11, _isr0x11, KERNEL_CS, 0x8E);
	i386_map_gate(0x12, _isr0x12, KERNEL_CS, 0x8E);
	i386_map_gate(0x13, _isr0x13, KERNEL_CS, 0x8E);
	i386_map_gate(0x14, _isr0x14, KERNEL_CS, 0x8E);
	i386_map_gate(0x15, _isr0x15, KERNEL_CS, 0x8E);
	i386_map_gate(0x16, _isr0x16, KERNEL_CS, 0x8E);
	i386_map_gate(0x17, _isr0x17, KERNEL_CS, 0x8E);
	i386_map_gate(0x18, _isr0x18, KERNEL_CS, 0x8E);
	i386_map_gate(0x19, _isr0x19, KERNEL_CS, 0x8E);
	i386_map_gate(0x1a, _isr0x1a, KERNEL_CS, 0x8E);
	i386_map_gate(0x1b, _isr0x1b, KERNEL_CS, 0x8E);
	i386_map_gate(0x1c, _isr0x1c, KERNEL_CS, 0x8E);
	i386_map_gate(0x1d, _isr0x1d, KERNEL_CS, 0x8E);
	i386_map_gate(0x1e, _isr0x1e, KERNEL_CS, 0x8E);
	i386_map_gate(0x1f, _isr0x1f, KERNEL_CS, 0x8E);

	/* Map all Interrupt Routines (IRQs) into the master IDT. */
	i386_map_gate(0x20, _irq0x20, KERNEL_CS, 0x8E);
	i386_map_gate(0x21, _irq0x21, KERNEL_CS, 0x8E);
	i386_map_gate(0x22, _irq0x22, KERNEL_CS, 0x8E);
	i386_map_gate(0x23, _irq0x23, KERNEL_CS, 0x8E);
	i386_map_gate(0x24, _irq0x24, KERNEL_CS, 0x8E);
	i386_map_gate(0x25, _irq0x25, KERNEL_CS, 0x8E);
	i386_map_gate(0x26, _irq0x26, KERNEL_CS, 0x8E);
	i386_map_gate(0x27, _irq0x27, KERNEL_CS, 0x8E);
	i386_map_gate(0x28, _irq0x28, KERNEL_CS, 0x8E);
	i386_map_gate(0x29, _irq0x29, KERNEL_CS, 0x8E);
	i386_map_gate(0x2a, _irq0x2a, KERNEL_CS, 0x8E);
	i386_map_gate(0x2b, _irq0x2b, KERNEL_CS, 0x8E);
	i386_map_gate(0x2c, _irq0x2c, KERNEL_CS, 0x8E);
	i386_map_gate(0x2d, _irq0x2d, KERNEL_CS, 0x8E);
	i386_map_gate(0x2e, _irq0x2e, KERNEL_CS, 0x8E);
	i386_map_gate(0x2f, _irq0x2f, KERNEL_CS, 0x8E);

	master_idt_ptr.size = sizeof(master_idt) - 1;
	master_idt_ptr.ptr = &master_idt;
	lidt(&master_idt_ptr);
	
	sti();
}

void interrupt_handler(struct i386_interrupt_frame *frame)
{
	if (frame->interrupt < 0x20) {
		/* Interrupt Service Routine received. */
		if (_int_handlers[frame->interrupt]) {
			_int_handlers[frame->interrupt](frame);
		}
		else {
			panic(
				"Unhandled Exception",
				"Exception %02X was received, but no handler for it was found. "
				"\n"
				" EAX: %08x   ESP: %08x   DS: %08x\n",
				" EBX: %08x   EBP: %08x   ES: %08x\n",
				" ECX: %08x   ESI: %08x   FS: %08x\n",
				" EDX: %08x   EDI: %08x   GS: %08x\n",
				"UESP: %08x   EIP: %08x   SS: %08x\n",
				" ERR: %08x   INT: %08x   CS: %08x\n",
				"EFLG: %08x\n",
				frame->interrupt,
				frame->eax, frame->esp, frame->ds,
				frame->ebx, frame->ebp, frame->es,
				frame->ecx, frame->esi, frame->fs,
				frame->edx, frame->edi, frame->gs, 
				frame->user_esp, frame->eip, frame->ss, 
				frame->errc, frame->interrupt, frame->cs, 
				frame->eflags
			);
		}
	}
	else if (frame->interrupt < 0x30) {
		/* Check for the appropriate IRQ handler */
		uint8_t irq = frame->interrupt - 0x20;

		if (_irq_handlers[irq]) {
			_irq_handlers[irq](irq);
		}

		/* Make sure the slave PIC is acknowledged before checking for a yield,
		   otherwise we'll miss the ACK. */
		if (frame->interrupt >= 0x28) {
			ack_slave_pic();
		}

		/* Check for a yield. */
		thread_yield((uintptr_t)frame, frame->ebp, irq);

		/* ACK the master PIC in case the yield failed. */
		ack_master_pic();
	}
	else {
		/* User defined interrupt received. */
		if (_int_handlers[frame->interrupt]) {
			_int_handlers[frame->interrupt](frame);
		}
	}
}

void set_irq_handler(uint8_t irq, irq_handler_t fn)
{
	irq -= 0x20;
	if (irq >= 0x0 && irq <= 0xF)
		_irq_handlers[irq] = fn;
}

void set_int_handler(uint8_t num, int_handler_t fn)
{
	_int_handlers[num] = fn;
}

#endif