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

#if !defined(DISPLAY_H)
#define DISPLAY_H

#include <types.h>
#include <arch.h>

/**
 The type of video display that can be configured in the system.

 - display_type_text	The display is operating in a text only mode.
 */
enum display_type
{
	display_type_text,
};

/**
 The `display_info` structure contains information and control functions for a
 given display. It acts as an abstraction layer around the internal specific
 functions of the display driver itself.

 - Note: This structure currently only supports a text mode display and will 
 need expanding in the future.

 - Important: When representing a text mode display, the dimensions and 
 positions are measured in terms of "character cells". When representing a none
 text mode dispay, the dimensions and positions will be measured in terms of
 "pixels" or "points".
 */
struct display_info
{
	/** 
	 The type of display being represented
	 */
	enum display_type type;

	/**
	 The current x-location of the cursor on the screen.
	 */
	uint32_t cursor_x;

	/**
	 The current y-location of the cursor on the screen.
	 */
	uint32_t cursor_y;

	/**
	 The width of the screen.
	 */
	uint32_t width;

	/**
	 The height of the screen.
	 */
	uint32_t height;

	/**
	 The current output attribute for the screen.
	 */
	uint32_t attribute;

	/**
	 Screen insets. This can be used to inset the perceived "edges" of the 
	 screen that is used for rendering, from the real physical edges.
	 */
	uint32_t inset_x;
	uint32_t inset_y;

	/**
	 Produce an attribute value for the specified RGB color value. This 
	 attribute value is one that will be compatible with the underlying display
	 driver.

	 - red 		The red component of the color
	 - green 	The green component of the color
	 - blue 	The blue component of the color
	 */
	uint32_t(*make_attribute)(uint32_t red, uint32_t green, uint32_t blue);

	/**
	 Put a character on the screen at the specified location and with the
	 specified attribute.

	 - Note: This only works for a text mode display.

	 - c 	The character to place on the screen
	 - x 	The x-location of the character
	 - y 	The y-location of the character
	 - attr The attribute of the character
	 */
	void(*putc)(const char c, uint32_t x, uint32_t y, uint32_t attr);

	/**
	 Clear the contents of the screen entirely.
	 */
	void(*clear)(void);

	/**
	 Set the cursor position on the screen.

	 - x 	The x-location of the cursor
	 - y 	The y-location of the cursor
	 */
	void(*set_cursor)(uint32_t x, uint32_t y);

	/**
	 Scroll the contents of the screen up by a single line.

	 - Note: This only works for a text mode display.
	 */
	void(*scroll)(void);

	/**
	 Set the default attribute of the screen. This affects the attribute used
	 when clearing the screen.
	 */
	void(*set_default_attribute)(uint32_t attr);

} __attribute__((packed));

/**
 A pointer to the display_info structure for the main display of the system.
 */
extern struct display_info *main_display;

/**
 A pointer to the output format information structure. This can be passed to the
 write_format() function to direct writing of format strings to the display.
 This serves as an alternate destination to `serial_out`.
 */
struct format_info *display_out;

/**
 Initialise the default display structures with information by the initial 
 display configuration.
 */
void init_display(void);

/**
 Clear all content from the main display.
 */
void display_clear(void);
void display_clear_text_range(uint32_t start_x, uint32_t start_y, uint32_t len);

/**
 Put a character to the main display (text mode displays only).

 - c 	The character to write to the display.
 */
void display_putc(const char c);

/**
 Put a string to the main display (text mode displays only).

 - str 	The string to write to the display.
 */
void display_puts(const char *restrict str);

/**
 Set the output attribute of the main display (text mode displays only).

 - attribute The attribute to use for output.
 */
void display_set_attribute(uint32_t attribute);

/**
 Set the inset values of the screen.
 */
void display_inset(uint32_t x, uint32_t y);

/**
 Get the current X,Y coordinates of the cursor.
 */
void display_get_cursor(uint32_t *x, uint32_t *y);

#endif