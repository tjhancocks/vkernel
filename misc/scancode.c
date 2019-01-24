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

#include <scancode.h>
#include <keycode.h>

////////////////////////////////////////////////////////////////////////////////

extern uint32_t __builtin_scancode_set_count(void);
extern struct scancode_info *default_scancode_set;

extern struct keycode_ascii_mapping *default_keycode_map;
extern uint32_t __builtin_keycode_map_count(void);

static uint8_t current_state = 0;
static enum key_modifiers current_modifiers = 0;
static enum key_state current_keystate = 0;

////////////////////////////////////////////////////////////////////////////////

oserr scancode_to_keyevent(struct keyevent *event, uint8_t scancode)
{
	if (scancode == '\0' || event == NULL) {
		return e_fail;
	}

	/* Set the initial default values */
	event->keycode = KC_ANSI_UNKNOWN;
	event->pressed = scancode & 0x80 ? false : true;

	/* Search for the scancode in the built in set */
	uint32_t count = __builtin_scancode_set_count();
	for (uint32_t n = 0; n < count; ++n) {
		if (default_scancode_set[n].state != current_state) {
			continue;
		}

		if (default_scancode_set[n].raw_code != (scancode & ~0x80)) {
			continue;
		}

		event->keycode = default_scancode_set[n].keycode;
		current_state = 0;

		enum key_modifiers new_modifier = 0;
		switch (event->keycode) {
			case KC_ANSI_LEFT_SHIFT:
				new_modifier = key_modifier_left_shift;
				break;
			case KC_ANSI_RIGHT_SHIFT:
				new_modifier = key_modifier_right_shift;
				break;
			case KC_ANSI_LEFT_CTRL:
				new_modifier = key_modifier_left_control;
				break;
			case KC_ANSI_RIGHT_CTRL:
				new_modifier = key_modifier_right_control;
				break;
			case KC_ANSI_LEFT_ALT:
				new_modifier = key_modifier_left_alt;
				break;
			case KC_ANSI_RIGHT_ALT:
				new_modifier = key_modifier_right_alt;
				break;
			case KC_ANSI_ESCAPE_CODE:
				current_state = KC_ANSI_ESCAPE_CODE;
				break;
		}

		if (event->pressed) {
			current_modifiers |= new_modifier;
		}
		else {
			current_modifiers &= ~new_modifier;
		}

		/* We've found what we're looking for */
		break;
	}

	event->modifiers = current_modifiers;
	event->state = current_keystate;
	return e_ok;
}

////////////////////////////////////////////////////////////////////////////////

oserr keyevent_to_ascii(struct keyevent *event, char *c)
{
	if (event == NULL || c == NULL) {
		return e_fail;
	}

	if (event->pressed == false) {
		return e_fail;
	}

	uint8_t keycode = event->keycode;
	enum key_modifiers modifiers = event->modifiers;

	uint32_t count = __builtin_keycode_map_count();
	for (uint32_t n = 0; n < count; ++n) {
		if (default_keycode_map[n].keycode != keycode) {
			continue;
		}

		if (modifiers & key_modifier_left_shift) {
			*c = default_keycode_map[n].left_shift;
		}
		else if (modifiers & key_modifier_right_shift) {
			*c = default_keycode_map[n].right_shift;
		}
		else if (modifiers & key_modifier_left_control) {
			*c = default_keycode_map[n].left_control;
		}
		else if (modifiers & key_modifier_right_control) {
			*c = default_keycode_map[n].right_control;
		}
		else if (modifiers & key_modifier_left_alt) {
			*c = default_keycode_map[n].left_alt;
		}
		else if (modifiers & key_modifier_right_alt) {
			*c = default_keycode_map[n].right_alt;
		}
		else {
			*c = default_keycode_map[n].base;
		}
		return e_ok;
	}

	return e_fail;
}