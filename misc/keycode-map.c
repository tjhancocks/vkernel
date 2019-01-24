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

struct keycode_ascii_mapping __builtin_keycode_map[] = 
{
	{KC_ANSI_1, '1', '!', '!', '1', '1', '1', '1'},
	{KC_ANSI_2, '2', '@', '@', '2', '2', '2', '2'},
	{KC_ANSI_3, '3', '#', '#', '3', '3', '3', '3'},
	{KC_ANSI_4, '4', '$', '$', '4', '4', '4', '4'},
	{KC_ANSI_5, '5', '%', '%', '5', '5', '5', '5'},
	{KC_ANSI_6, '6', '^', '^', '6', '6', '6', '6'},
	{KC_ANSI_7, '7', '&', '&', '7', '7', '7', '7'},
	{KC_ANSI_8, '8', '*', '*', '8', '8', '8', '8'},
	{KC_ANSI_9, '9', '(', '(', '9', '9', '9', '9'},
	{KC_ANSI_0, '0', ')', ')', '0', '0', '0', '0'},
	{KC_ANSI_MINUS, '-', '_', '_', '-', '-', '-', '-'},
	{KC_ANSI_EQUALS, '=', '+', '+', '=', '=', '=', '='},
	{KC_ANSI_BACKSPACE, '\b', '\b', '\b', '\b', '\b', '\b', '\b'},
	{KC_ANSI_NUM_SLASH, '/', '/', '/', '/', '/', '/', '/'},
	{KC_ANSI_NUM_STAR, '*', '*', '*', '*', '*', '*', '*'},
	{KC_ANSI_NUM_MINUS, '-', '-', '-', '-', '-', '-', '-'},
	{KC_ANSI_TAB, '\t', '\t', '\t', '\t', '\t', '\t', '\t'},
	{KC_ANSI_Q, 'q', 'Q', 'Q', 'q', 'q', 'q', 'q'},
	{KC_ANSI_W, 'w', 'W', 'W', 'w', 'w', 'w', 'w'},
	{KC_ANSI_E, 'e', 'E', 'E', 'e', 'e', 'e', 'e'},
	{KC_ANSI_R, 'r', 'R', 'R', 'r', 'r', 'r', 'r'},
	{KC_ANSI_T, 't', 'T', 'T', 't', 't', 't', 't'},
	{KC_ANSI_Y, 'y', 'Y', 'Y', 'y', 'y', 'y', 'y'},
	{KC_ANSI_U, 'u', 'U', 'U', 'u', 'u', 'u', 'u'},
	{KC_ANSI_I, 'i', 'I', 'I', 'i', 'i', 'i', 'i'},
	{KC_ANSI_O, 'o', 'O', 'O', 'o', 'o', 'o', 'o'},
	{KC_ANSI_P, 'p', 'P', 'P', 'p', 'p', 'p', 'p'},
	{KC_ANSI_LEFT_BRACKET, '[', '{', '{', '[', '[', '[', '['},
	{KC_ANSI_RIGHT_BRACKET, ']', '}', '}', ']', ']', ']', ']'},
	{KC_ANSI_NUM_ENTER, '\n', '\n', '\n', '\n', '\n', '\n', '\n'},
	{KC_ANSI_DEL, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F},
	{KC_ANSI_NUM_7, '7', '7', '7', '7', '7', '7', '7'},
	{KC_ANSI_NUM_8, '8', '8', '8', '8', '8', '8', '8'},
	{KC_ANSI_NUM_9, '9', '9', '9', '9', '9', '9', '9'},
	{KC_ANSI_NUM_PLUS, '+', '+', '+', '+', '+', '+', '+'},
	{KC_ANSI_A, 'a', 'A', 'A', 'a', 'a', 'a', 'a'},
	{KC_ANSI_S, 's', 'S', 'S', 's', 's', 's', 's'},
	{KC_ANSI_D, 'd', 'D', 'D', 'd', 'd', 'd', 'd'},
	{KC_ANSI_F, 'f', 'F', 'F', 'f', 'f', 'f', 'f'},
	{KC_ANSI_G, 'g', 'G', 'G', 'g', 'g', 'g', 'g'},
	{KC_ANSI_H, 'h', 'H', 'H', 'h', 'h', 'h', 'h'},
	{KC_ANSI_J, 'j', 'J', 'J', 'j', 'j', 'j', 'j'},
	{KC_ANSI_K, 'k', 'K', 'K', 'k', 'k', 'k', 'k'},
	{KC_ANSI_L, 'l', 'L', 'L', 'l', 'l', 'l', 'l'},
	{KC_ANSI_SEMI_COLON, ';', ':', ':', ';', ';', ';', ';'},
	{KC_ANSI_QUOTE, '\'', '"', '"', '\'', '\'', '\'', '\''},
	{KC_ANSI_HASH, '#', '#', '#', '#', '#', '#', '#'},
	{KC_ANSI_NUM_4, '4', '4', '4', '4', '4', '4', '4'},
	{KC_ANSI_NUM_5, '5', '5', '5', '5', '5', '5', '5'},
	{KC_ANSI_NUM_6, '6', '6', '6', '6', '6', '6', '6'},
	{KC_ANSI_ENTER, '\n', '\n', '\n', '\n', '\n', '\n', '\n'},
	{KC_ANSI_BACKSLASH, '\\', '\\', '\\', '\\', '\\', '\\', '\\'},
	{KC_ANSI_Z, 'z', 'Z', 'Z', 'z', 'z', 'z', 'z'},
	{KC_ANSI_X, 'x', 'X', 'X', 'x', 'x', 'x', 'x'},
	{KC_ANSI_C, 'c', 'C', 'C', 'c', 'c', 'c', 'c'},
	{KC_ANSI_V, 'v', 'V', 'V', 'v', 'v', 'v', 'v'},
	{KC_ANSI_B, 'b', 'B', 'B', 'b', 'b', 'b', 'b'},
	{KC_ANSI_N, 'n', 'N', 'N', 'n', 'n', 'n', 'n'},
	{KC_ANSI_M, 'm', 'M', 'M', 'm', 'm', 'm', 'm'},
	{KC_ANSI_COMMA, ',', '<', '<', ',', ',', ',', ','},
	{KC_ANSI_PERIOD, '.', '>', '>', '.', '.', '.', '.'},
	{KC_ANSI_SLASH, '/', '?', '?', '/', '/', '/', '/'},
	{KC_ANSI_NUM_1, '1', '1', '1', '1', '1', '1', '1'},
	{KC_ANSI_NUM_2, '2', '2', '2', '2', '2', '2', '2'},
	{KC_ANSI_NUM_3, '3', '3', '3', '3', '3', '3', '3'},
	{KC_ANSI_SPACE, ' ', ' ', ' ', ' ', ' ', ' ', ' '},
	{KC_ANSI_NUM_0, '0', '0', '0', '0', '0', '0', '0'},
	{KC_ANSI_NUM_PERIOD, '.', '.', '.', '.', '.', '.', '.'},
};
struct keycode_ascii_mapping *default_keycode_map = &__builtin_keycode_map;

uint32_t __builtin_keycode_map_count(void)
{
	return (uint32_t)(
		sizeof(__builtin_keycode_map) / sizeof(struct keycode_ascii_mapping)
	);
}