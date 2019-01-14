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

#include <stack.h>
#include <arch.h>
#include <print.h>

extern void __thread_start(void);

oserr init_stack(void *restrict stack, void(*start)(void), uint32_t *sp)
{
	/* The stack needs to be correctly populated so that when the scheduler
	   first switches execution to it, it does not crash and behaves as 
	   expected.

	   However because we switch to new threads via interrupts, we also need
	   to setup an interrupt frame so that the scheduler does not crash when
	   it returns from the interrupt. */

	uint32_t *ebp = stack;
	uint32_t *esp = ebp - 1;

	/* Setup the starting point. These are parameters to the__ thread_start()
	   function. */
	*esp-- = (uintptr_t)start;
	*esp-- = 0x00;

	/* Setup the interrupt frame. */
	*esp-- = KERNEL_DS;					/* interrupt_frame.ss */
	*esp-- = 0x00;						/* interrupt_frame.user_esp */
	*esp-- = 0x208;						/* interrupt_frame.eflags */
	*esp-- = KERNEL_CS;					/* interrupt_frame.cs */
	*esp-- = (uintptr_t)__thread_start;	/* interrupt_frame.eip */
	*esp-- = 0x09;						/* interrupt_frame.errc */
	*esp-- = 0x08;						/* interrupt_frame.interrupt */
	*esp-- = 0x01;						/* interrupt_frame.eax */
	*esp-- = 0x02;						/* interrupt_frame.ecx */
	*esp-- = 0x03;						/* interrupt_frame.edx */
	*esp-- = 0x04;						/* interrupt_frame.ebx */
	*esp-- = 0x05;						/* interrupt_frame.esp (ignored) */
	*esp-- = (uintptr_t)ebp;			/* interrupt_frame.ebp */
	*esp-- = 0x06;						/* interrupt_frame.esi */
	*esp-- = 0x07;						/* interrupt_frame.edi */
	*esp-- = KERNEL_DS;					/* interrupt_frame.ds */
	*esp-- = KERNEL_DS;					/* interrupt_frame.es */
	*esp-- = KERNEL_DS;					/* interrupt_frame.fs */
	*esp   = KERNEL_DS;					/* interrupt_frame.gs */

	/* Make sure that the correct stack pointer is passed back to the caller */
	if (!sp) {
		klogc(swarn, "Stack has been setup, but stack pointer ignored.\n");
		return e_fail;
	}
	*sp = esp;
	klogc(sinfo, "stack created: ebp=%p, esp=%p, start=%p\n", 
		ebp, esp, __thread_start);
	
	/* report a success */
	return e_ok;
}