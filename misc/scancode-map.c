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

struct scancode_info __builtin_scancode_set[] = {
	{0x01, KC_ANSI_ESC, 0x00, 0x00, "escape pressed"},
	{0x02, KC_ANSI_1, 0x00, 0x00, "1 pressed"},
	{0x03, KC_ANSI_2, 0x00, 0x00, "2 pressed"},
	{0x04, KC_ANSI_3, 0x00, 0x00, "3 pressed"},
	{0x05, KC_ANSI_4, 0x00, 0x00, "4 pressed"},
	{0x06, KC_ANSI_5, 0x00, 0x00, "5 pressed"},
	{0x07, KC_ANSI_6, 0x00, 0x00, "6 pressed"},
	{0x08, KC_ANSI_7, 0x00, 0x00, "7 pressed"},
	{0x09, KC_ANSI_8, 0x00, 0x00, "8 pressed"},
	{0x0A, KC_ANSI_9, 0x00, 0x00, "9 pressed"},
	{0x0B, KC_ANSI_0, 0x00, 0x00, "0 pressed"},
	{0x0C, KC_ANSI_MINUS, 0x00, 0x00, "minus pressed"},
	{0x0D, KC_ANSI_EQUALS, 0x00, 0x00, "equals pressed"},
	{0x0E, KC_ANSI_BACKSPACE, 0x00, 0x00, "backspace pressed"},
	{0x0F, KC_ANSI_TAB, 0x00, 0x00, "tab pressed"},
	{0x10, KC_ANSI_Q, 0x00, 0x00, "Q pressed"},
	{0x11, KC_ANSI_W, 0x00, 0x00, "W pressed"},
	{0x12, KC_ANSI_E, 0x00, 0x00, "E pressed"},
	{0x13, KC_ANSI_R, 0x00, 0x00, "R pressed"},
	{0x14, KC_ANSI_T, 0x00, 0x00, "T pressed"},
	{0x15, KC_ANSI_Y, 0x00, 0x00, "Y pressed"},
	{0x16, KC_ANSI_U, 0x00, 0x00, "U pressed"},
	{0x17, KC_ANSI_I, 0x00, 0x00, "I pressed"},
	{0x18, KC_ANSI_O, 0x00, 0x00, "O pressed"},
	{0x19, KC_ANSI_P, 0x00, 0x00, "P pressed"},
	{0x1A, KC_ANSI_LEFT_BRACKET, 0x00, 0x00, "left bracket pressed"},
	{0x1B, KC_ANSI_RIGHT_BRACKET, 0x00, 0x00, "right bracket pressed"},
	{0x1C, KC_ANSI_ENTER, 0x00, 0x00, "enter pressed"},
	{0x1D, KC_ANSI_LEFT_CTRL, 0x00, 0x00, "left control pressed"},
	{0x1E, KC_ANSI_A, 0x00, 0x00, "A pressed"},
	{0x1F, KC_ANSI_S, 0x00, 0x00, "S pressed"},
	{0x20, KC_ANSI_D, 0x00, 0x00, "D pressed"},
	{0x21, KC_ANSI_F, 0x00, 0x00, "F pressed"},
	{0x22, KC_ANSI_G, 0x00, 0x00, "G pressed"},
	{0x23, KC_ANSI_H, 0x00, 0x00, "H pressed"},
	{0x24, KC_ANSI_J, 0x00, 0x00, "J pressed"},
	{0x25, KC_ANSI_K, 0x00, 0x00, "K pressed"},
	{0x26, KC_ANSI_L, 0x00, 0x00, "L pressed"},
	{0x27, KC_ANSI_SEMI_COLON, 0x00, 0x00, "semi colon pressed"},
	{0x28, KC_ANSI_QUOTE, 0x00, 0x00, "quote pressed"},
	{0x29, KC_ANSI_BK_TICK, 0x00, 0x00, "back tick pressed"},
	{0x2A, KC_ANSI_LEFT_SHIFT, 0x00, 0x00, "left shift pressed"},
	{0x2B, KC_ANSI_BACKSLASH, 0x00, 0x00, "backslash pressed"},
	{0x2C, KC_ANSI_Z, 0x00, 0x00, "Z pressed"},
	{0x2D, KC_ANSI_X, 0x00, 0x00, "X pressed"},
	{0x2E, KC_ANSI_C, 0x00, 0x00, "C pressed"},
	{0x2F, KC_ANSI_V, 0x00, 0x00, "V pressed"},
	{0x30, KC_ANSI_B, 0x00, 0x00, "B pressed"},
	{0x31, KC_ANSI_N, 0x00, 0x00, "N pressed"},
	{0x32, KC_ANSI_M, 0x00, 0x00, "M pressed"},
	{0x33, KC_ANSI_COMMA, 0x00, 0x00, "comma pressed"},
	{0x34, KC_ANSI_PERIOD, 0x00, 0x00, "period pressed"},
	{0x35, KC_ANSI_SLASH, 0x00, 0x00, "slash pressed"},
	{0x36, KC_ANSI_RIGHT_SHIFT, 0x00, 0x00, "right shift pressed"},
	{0x37, KC_ANSI_NUM_STAR, 0x00, 0x00, "keypad star pressed"},
	{0x38, KC_ANSI_LEFT_ALT, 0x00, 0x00, "left alt pressed"},
	{0x39, KC_ANSI_SPACE, 0x00, 0x00, "space pressed"},
	{0x3A, KC_ANSI_CAPS_LOCK, 0x00, 0x00, "caps lock pressed"},
	{0x3B, KC_ANSI_F1, 0x00, 0x00, "F1 pressed"},
	{0x3C, KC_ANSI_F2, 0x00, 0x00, "F2 pressed"},
	{0x3D, KC_ANSI_F3, 0x00, 0x00, "F3 pressed"},
	{0x3E, KC_ANSI_F4, 0x00, 0x00, "F4 pressed"},
	{0x3F, KC_ANSI_F5, 0x00, 0x00, "F5 pressed"},
	{0x40, KC_ANSI_F6, 0x00, 0x00, "F6 pressed"},
	{0x41, KC_ANSI_F7, 0x00, 0x00, "F7 pressed"},
	{0x42, KC_ANSI_F8, 0x00, 0x00, "F8 pressed"},
	{0x43, KC_ANSI_F9, 0x00, 0x00, "F9 pressed"},
	{0x44, KC_ANSI_F10, 0x00, 0x00, "F10 pressed"},
	{0x45, KC_ANSI_NUM_LOCK, 0x00, 0x00, "number lock pressed"},
	{0x46, KC_ANSI_SCROLL_LOCK, 0x00, 0x00, "scroll lock pressed"},
	{0x47, KC_ANSI_NUM_7, 0x00, 0x00, "keypad 7 pressed"},
	{0x48, KC_ANSI_NUM_8, 0x00, 0x00, "keypad 8 pressed"},
	{0x49, KC_ANSI_NUM_9, 0x00, 0x00, "keypad 9 pressed"},
	{0x4A, KC_ANSI_NUM_MINUS, 0x00, 0x00, "keypad minus pressed"},
	{0x4B, KC_ANSI_NUM_4, 0x00, 0x00, "keypad 4 pressed"},
	{0x4C, KC_ANSI_NUM_5, 0x00, 0x00, "keypad 5 pressed"},
	{0x4D, KC_ANSI_NUM_9, 0x00, 0x00, "keypad 6 pressed"},
	{0x4E, KC_ANSI_NUM_PLUS, 0x00, 0x00, "keypad plus pressed"},
	{0x4F, KC_ANSI_NUM_1, 0x00, 0x00, "keypad 1 pressed"},
	{0x50, KC_ANSI_NUM_2, 0x00, 0x00, "keypad 2 pressed"},
	{0x51, KC_ANSI_NUM_3, 0x00, 0x00, "keypad 3 pressed"},
	{0x52, KC_ANSI_NUM_0, 0x00, 0x00, "keypad 0 pressed"},
	{0x53, KC_ANSI_NUM_PERIOD, 0x00, 0x00, "keypad dot pressed"},
	{0x57, KC_ANSI_F11, 0x00, 0x00, "F11 pressed"},
	{0x58, KC_ANSI_F12, 0x00, 0x00, "F12 pressed"},
	{0xE0, KC_ANSI_ESCAPE_CODE, 0x00, 0xE0, "(escape code)"},
};

struct scancode_info *default_scancode_set = &__builtin_scancode_set;

uint32_t __builtin_scancode_set_count(void)
{
	return (uint32_t)(
		sizeof(__builtin_scancode_set) / sizeof(struct scancode_info)
	);
}