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
#include <print.h>
#include <format.h>

#define TAB_SIZE 4

////////////////////////////////////////////////////////////////////////////////

struct format_info _display_format = {
	.out = display_puts,
};
struct format_info *display_out = &_display_format;

////////////////////////////////////////////////////////////////////////////////

static inline int __chk_display_type(enum display_type type)
{
	return (main_display && type == main_display->type);
}

////////////////////////////////////////////////////////////////////////////////

void init_display(void)
{
	/* TODO: This should initialise the data structures accordingly for the
	   required type of display.
	   This is currently a stub as we assume VGA Text Mode on i386 right now. */
	klogc(sinfo, "*** Display is currently assuming VGA Text Mode.\n");
}

void display_clear(void)
{
	if (main_display && main_display->clear)
		main_display->clear();
}

void display_putc(const char c)
{
	if (!__chk_display_type(display_type_text))
		return;

	if (main_display->putc) {
		
		if (c < ' ') {
			/* The character isn't directly printable and is technically a
			   "control" character. */
			switch (c) {
				case '\0': /* NUL */
					break;
				case '\t': /* Horizontal tab */
					main_display->cursor_x += TAB_SIZE;
					main_display->cursor_x &= ~(TAB_SIZE - 1);
					break;
				case '\n': /* Line feed */
					++main_display->cursor_y;
				case '\r': /* Carridge return */
					main_display->cursor_x = main_display->inset_x;
					break;
				case '\a': /* Bell. Should emit a sound. */
					/* TODO: Produce a beep sound */
					break;
				case '\b': /* Backspace. */
					main_display->putc(
						' ', 
						--main_display->cursor_x, 
						main_display->cursor_y, 
						main_display->attribute
					);
					break;
				default:
					break;
			}
		}
		else if (c >= ' ') {
			/* The character is a printable one. Just print it to screen. */
			main_display->putc(
				c, 
				main_display->cursor_x++, 
				main_display->cursor_y, 
				main_display->attribute
			);
		}
	}

	uint32_t effective_width = (
		main_display->width - (main_display->inset_x << 1)
	);
	uint32_t effective_height = (
		main_display->height - (main_display->inset_y << 1)
	);

	if (main_display->cursor_x >= effective_width) {
		main_display->cursor_x = main_display->inset_x;
		++main_display->cursor_y;
	}
	if (main_display->cursor_y >= effective_height) {
		if (main_display->scroll)
			main_display->scroll();
		--main_display->cursor_y;
	}
}

void display_puts(const char *restrict str)
{
	while (*str)
		display_putc(*str++);

	if (main_display && main_display->set_cursor)
		main_display->set_cursor(
			main_display->cursor_x, main_display->cursor_y
		);
}

void display_set_attribute(uint32_t attribute)
{
	if (main_display) {
		main_display->attribute = attribute;
	}
}

void display_inset(uint32_t x, uint32_t y)
{
	if (main_display) {
		main_display->inset_x = main_display->cursor_x = x;
		main_display->inset_y = main_display->cursor_y = y;
	}
}