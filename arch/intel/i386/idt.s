; Copyright (c) 2018-2019 Tom Hancocks
; 
; Permission is hereby granted, free of charge, to any person obtaining a copy
; of this software and associated documentation files (the "Software"), to deal
; in the Software without restriction, including without limitation the rights
; to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
; copies of the Software, and to permit persons to whom the Software is
; furnished to do so, subject to the following conditions:
; 
; The above copyright notice and this permission notice shall be included in all
; copies or substantial portions of the Software.
; 
; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
; AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
; LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
; OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
; SOFTWARE.

	bits          32

	extern        interrupt_handler

%macro isr_noerr 1
	section .text
	global  _isr%1
_isr%1:
	cli
	push	byte 0
	push 	byte %1
	jmp 	_interrupt_trampoline
%endmacro

%macro isr_err 1
	section .text
	global  _isr%1
_isr%1:
	cli
	push 	byte %1
	jmp 	_interrupt_trampoline
%endmacro

%macro irq 1
	section .text
	global  _irq%1
_irq%1:
	cli
	push 	byte 0
	push 	byte %1
	jmp 	_interrupt_trampoline
%endmacro

; Generate all of the Interrupt Service Routine (ISR) stubs
isr_noerr 0x00
isr_noerr 0x01
isr_noerr 0x02
isr_noerr 0x03
isr_noerr 0x04
isr_noerr 0x05
isr_noerr 0x06
isr_noerr 0x07
isr_err   0x08
isr_noerr 0x09
isr_err   0x0a
isr_err   0x0b
isr_err   0x0c
isr_err   0x0d
isr_err   0x0e
isr_noerr 0x0f
isr_noerr 0x10
isr_err   0x11
isr_noerr 0x12
isr_noerr 0x13
isr_noerr 0x14
isr_noerr 0x15
isr_noerr 0x16
isr_noerr 0x17
isr_noerr 0x18
isr_noerr 0x19
isr_noerr 0x1a
isr_noerr 0x1b
isr_noerr 0x1c
isr_noerr 0x1d
isr_noerr 0x1e
isr_noerr 0x1f

; Generate all of the Interrupt Request (IRQ) stubs
irq       0x20
irq       0x21
irq       0x22
irq       0x23
irq       0x24
irq       0x25
irq       0x26
irq       0x27
irq       0x28
irq       0x29
irq       0x2a
irq       0x2b
irq       0x2c
irq       0x2d
irq       0x2e
irq       0x2f

; The Interrupt Trampoline responsible for getting control back into the C
; layer.
	section .text
_interrupt_trampoline:
	pusha
	push 	ds
	push 	es
	push 	fs
	push 	gs
	mov 	ax, 0x10
	mov 	ds, ax
	mov 	es, ax
	mov 	fs, ax
	mov 	gs, ax
	cld
	push 	esp
	call 	interrupt_handler
	add 	esp, 0x04
	pop 	gs
	pop 	fs
	pop 	es
	pop 	ds
	popa
	add 	esp, 0x08
	iret
