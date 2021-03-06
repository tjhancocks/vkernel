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

#if !defined(SCANCODE_H)
#define SCANCODE_H

#include <types.h>

enum key_modifiers 
{
	key_modifier_left_shift = 1 << 1,
	key_modifier_right_shift = 1 << 2,
	key_modifier_left_control = 1 << 3,
	key_modifier_right_control = 1 << 4,
	key_modifier_left_alt = 1 << 5,
	key_modifier_right_alt = 1 << 6,
};

enum key_state
{
	key_state_caps_lock = 1 << 1,
	key_state_num_lock = 1 << 2,
	key_state_scroll_lock = 1 << 3
};

struct scancode_info
{
	uint8_t raw_code;
	uint8_t keycode;
	uint8_t state;
	uint8_t set_state;
	const char *name;
};

struct keyevent
{
	uint8_t keycode;
	bool pressed;
	enum key_modifiers modifiers;
	enum key_state state;
};

struct keycode_ascii_mapping 
{
	uint8_t keycode;
	char base;
	char left_shift;
	char right_shift;
	char left_control;
	char right_control;
	char left_alt;
	char right_alt;
};

oserr scancode_to_keyevent(struct keyevent *event, uint8_t scancode);
oserr keyevent_to_ascii(struct keyevent *event, char *c);

#endif