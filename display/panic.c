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

#include <display.h>
#include <vargs.h>
#include <arch.h>
#include <print.h>

static void panic_text_va(
	const char *restrict title, 
	const char *restrict message, 
	va_list va
) {
	/* We're operating inside a text mode display. We can make assumptions
	   here. */

	/* Use a classic blue background, white text, BSOD */
	display_set_attribute(0x1F);
	display_clear();

	/* Inset the entire screen as we do not want to be flush against the edges
	   for this. */
	display_inset(2, 1);

	/* Render the title */
	kprint("%s\n", title);

	/* Render the message */
	display_set_attribute(0x19);
	kprintc_va(snone, message, va);
}

__attribute__((noreturn)) static void panic_va(
	const char *restrict title,
	const char *restrict message,
	va_list va
) {
	/* The rendering of the screen needs to be done by an appropriate
	   function for the type of screen we're dealing with. */
	switch (main_display->type) {
		case display_type_text:
		default:
			panic_text_va(title, message, va);
			break;
	}

	/* Halt the system and go into an infinite loop as a safety measure. */
	for (;;)
		hlt();
}

__attribute__((noreturn)) void panic(
	const char *restrict title,
	const char *restrict message,...
) {
	va_list va;
	va_start(va, message);
	panic_va(title, message, va);
}
