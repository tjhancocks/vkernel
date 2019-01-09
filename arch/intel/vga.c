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
#include <display.h>

static enum vga_mode __vga_mode = vga_text_mode;
static uint32_t __vga_width = 80;
static uint32_t __vga_height = 25;
static void *__vga_vidmem = (void *)0xB8000;
static uint8_t __vga_char_attribute = 0x07;

struct display_info __text_mode_display = {
	.type = display_type_text,
	.cursor_x = 0,
	.cursor_y = 0,
	.make_attribute = vga_make_attribute,
	.putc = vga_put_char,
	.clear = vga_clear,
	.set_cursor = vga_set_cursor,
	.scroll = vga_scroll,
};

struct display_info *main_display = &__text_mode_display;

////////////////////////////////////////////////////////////////////////////////

static inline uint16_t __vga_char_cell(const char c, uint16_t attribute)
{
	return (attribute << 8) | c;
}

static inline uint32_t __vga_offset_text_mode(uint32_t x, uint32_t y)
{
	return (y * __vga_width) + x;
}

static inline void __vga_set_cursor_text_mode(uint32_t x, uint32_t y)
{
	register uint16_t pos = (uint16_t)__vga_offset_text_mode(x, y);
	outb(0x3D4, 0x0F);
	outb(0x3D5, (uint8_t)(pos & 0xFF));
	outb(0x3D4, 0x0E);
	outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

static inline void __vga_clear_text_mode(void)
{
	register uint16_t *ptr = __vga_vidmem;
	register uint32_t size = (__vga_width * __vga_height);
	for (register uint32_t offset = 0; offset < size; ++offset) {
		ptr[offset] = __vga_char_cell(' ', __text_mode_display.attribute);
	}
	__vga_set_cursor_text_mode(0, 0);

	if (__text_mode_display.width == 0 && __text_mode_display.height == 0) {
		__text_mode_display.width = __vga_width;
		__text_mode_display.height = __vga_height;
	}
}

static inline void __vga_put_char_text_mode(
	const char c, uint8_t x, uint8_t y, uint8_t attribute
) {
	register uint16_t *ptr = __vga_vidmem;
	register uint16_t pos = (uint16_t)__vga_offset_text_mode(x, y);
	ptr[pos] = __vga_char_cell(c, attribute);
}

static inline void __vga_scroll_text_mode(void)
{
	register uint16_t *ptr = __vga_vidmem;
	register uint32_t size = (__vga_width * (__vga_height - 1));
	for (register uint32_t offset = 0; offset < size; ++offset) {
		ptr[offset] = ptr[offset + __vga_width];
	}
	for (register uint32_t offset = 0; offset < __vga_width; ++offset) {
		ptr[offset + size] = __vga_char_cell(' ', __vga_char_attribute);
	}
}

////////////////////////////////////////////////////////////////////////////////

void init_vga(void)
{
	/* TODO: The width and height should really be checked. It is possible that
	   we could have been placed in a different video mode to the default. */
	__text_mode_display.width = __vga_width;
	__text_mode_display.height = __vga_height;
	__text_mode_display.attribute = __vga_char_attribute;

	/* Ensure the display is clear. */
	vga_clear();
}

void vga_clear(void)
{
	switch (__vga_mode) {
		case vga_text_mode:
			__vga_clear_text_mode();
			break;
		default:
			break;
	}
}

void vga_put_char(const char c, uint32_t x, uint32_t y, uint8_t attribute)
{
	switch (__vga_mode) {
		case vga_text_mode:
			__vga_put_char_text_mode(c, x, y, attribute);
			break;
		default:
			break;
	}
}

void vga_set_cursor(uint32_t x, uint32_t y)
{
	switch (__vga_mode) {
		case vga_text_mode:
			__vga_set_cursor_text_mode(x, y);
			break;
		default:
			break;
	}
}

uint32_t vga_make_attribute(uint32_t r, uint32_t g, uint32_t b)
{
	uint32_t luma = (r+r+r+b+g+g+g+g)>>3;
	return (luma >= 128 ? 0x8 : 0x0)
		| (r >= 128 ? 0x4 : 0x0)
		| (g >= 128 ? 0x2 : 0x0)
		| (b >= 128 ? 0x1 : 0x0);
}

void vga_scroll(void)
{
	switch (__vga_mode) {
		case vga_text_mode:
			__vga_scroll_text_mode();
			break;
		default:
			break;
	}
}

#endif