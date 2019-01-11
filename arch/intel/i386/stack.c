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

static void _stub_eip(void *a, uint32_t b)
{
	kprint("Started stub thread.\n");
	for (;;) {
		hang();
	}
}

oserr init_stack(void *restrict stack, void(*start)(void), uint32_t *sp)
{
	/* The stack needs to be correctly populated so that when the scheduler
	   first switches execution to it, it does not crash and behaves as 
	   expected.

	   However because we switch to new threads via interrupts, we also need
	   to setup an interrupt frame so that the scheduler does not crash when
	   it returns from the interrupt. */

	uint32_t esp = 0;
	uint32_t *ebp = stack;

	/* Setup the starting point. These are parameters to the thread_start()
	   function. */
	ebp[esp++] = (uintptr_t)start;
	ebp[esp++] = 0x00;

	/* Setup the interrupt frame. */
	ebp[esp++] = KERNEL_DS;					/* interrupt_frame.ss */
	ebp[esp++] = 0x00;						/* interrupt_frame.user_esp */
	ebp[esp++] = 0x208;						/* interrupt_frame.eflags */
	ebp[esp++] = KERNEL_CS;					/* interrupt_frame.cs */
	ebp[esp++] = (uintptr_t)_stub_eip;		/* interrupt_frame.eip */
	ebp[esp++] = 0x00;						/* interrupt_frame.errc */
	ebp[esp++] = 0x00;						/* interrupt_frame.interrupt */
	ebp[esp++] = 0x00;						/* interrupt_frame.edi */
	ebp[esp++] = 0x00;						/* interrupt_frame.esi */
	ebp[esp++] = (uintptr_t)ebp;			/* interrupt_frame.ebp */
	ebp[esp++] = 0x00;						/* interrupt_frame.esp (ignored) */
	ebp[esp++] = 0x00;						/* interrupt_frame.ebx */
	ebp[esp++] = 0x00;						/* interrupt_frame.edx */
	ebp[esp++] = 0x00;						/* interrupt_frame.ecx */
	ebp[esp++] = 0x00;						/* interrupt_frame.eax */
	ebp[esp++] = KERNEL_DS;					/* interrupt_frame.ds */
	ebp[esp++] = KERNEL_DS;					/* interrupt_frame.es */
	ebp[esp++] = KERNEL_DS;					/* interrupt_frame.fs */
	ebp[esp]   = KERNEL_DS;					/* interrupt_frame.gs */

	/* Make sure that the correct stack pointer is passed back to the caller */
	if (!sp) {
		kprintc(swarn, "Stack has been setup, but stack pointer ignored.\n");
		return e_fail;
	}
	*sp = esp;
	
	/* report a success */
	return e_ok;
}