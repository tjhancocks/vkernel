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

#if !defined(VGA_H) && (__i386__ || __x86_64__)
#define VGA_H

enum vga_color
{
	vga_black = 0x0,
	vga_blue,
	vga_green,
	vga_cyan,
	vga_red,
	vga_magenta,
	vga_brown,
	vga_grey,
	vga_dgrey,
	vga_lblue,
	vga_lgreen,
	vga_lcyan,
	vga_lred,
	vga_lpink,
	vga_yellow,
	vga_white,
};

enum vga_mode
{
	vga_text_mode,
};

void init_vga(void);
void vga_clear(void);
void vga_put_char(const char c, uint32_t x, uint32_t y, uint8_t attribute);
void vga_set_cursor(uint32_t x, uint32_t y);
uint32_t vga_make_attribute(uint32_t r, uint32_t g, uint32_t b);
void vga_scroll(void);

#endif