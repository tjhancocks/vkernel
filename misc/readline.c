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

#include <read.h>
#include <keyboard.h>
#include <scancode.h>
#include <display.h>
#include <sound.h>

uint32_t readline(char *restrict buffer, uint32_t length)
{
	uint32_t start_x, start_y;
	display_get_cursor(&start_x, &start_y);

	uint32_t ptr = 0;
	uint32_t max = 0;

	/* Listen for key stroke inputs until we get a new line */
	do {
		/* Read/wait for a keyevent */
		struct keyevent event = { 0 };
		char c;
		if (scancode_to_keyevent(&event, keyboard_read_scancode()) == e_fail) {
			/* invalid scancode, wait again */
			continue;
		}
		if (keyevent_to_ascii(&event, &c) == e_fail) {
			/* invalid keyevent, wait again */
			continue;	
		}

		/* we got a valid keyevent */
		if (c == '\b') {
			/* backspace needs to be handled differently */
			if (ptr > 0) {
				buffer[--ptr] = '\0';
			}
			else {
				beep();
			}
		}
		else if (c == '\n') {
			/* enter needs to be handled differently */
			break;
		}
		else if (ptr < length - 1) {
			buffer[ptr++] = c;
			max++;
		}
		else {
			/* unable to accept input */
			beep();
		}

		/* Render the buffer. We need to first clear it and then draw in the
	       new buffer. */
		display_clear_text_range(start_x, start_y, max);
		display_puts(buffer);
	}
	while (true);
}