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

#include <keyboard.h>
#include <vmm.h>
#include <print.h>
#include <thread.h>

////////////////////////////////////////////////////////////////////////////////

static uint8_t *_kbd_buffer;
static uint32_t _kbd_buffer_len;
static uint32_t _kbd_buffer_in;
static uint32_t _kbd_buffer_out;

////////////////////////////////////////////////////////////////////////////////

void init_keyboard(void)
{
	/* Setup the buffer. This should ideally be done by acquiring a specific
	   frame and assigning it to a commonly used virtual address across all
	   processes. */
	_kbd_buffer = vmm_acquire_any_page();
	klogc(sok, "Acquired page %p for keyboard buffer.\n", _kbd_buffer);

	_kbd_buffer_len = 32;
	_kbd_buffer_in = 0;
	_kbd_buffer_out = 0;
}

////////////////////////////////////////////////////////////////////////////////

static inline uint32_t keyboard_buffer_item_count(void)
{
	if (_kbd_buffer_in > _kbd_buffer_out) {
		return (_kbd_buffer_in - _kbd_buffer_out);
	}
	else if (_kbd_buffer_out == _kbd_buffer_in) {
		/* There are no items in the buffer! */
		return 0;
	}
	else {
		return _kbd_buffer_len + (_kbd_buffer_in - _kbd_buffer_out);
	}
}

bool keyboard_has_items(void)
{
	return keyboard_buffer_item_count() > 0;
}

////////////////////////////////////////////////////////////////////////////////

void keyboard_record_scancode(uint8_t scancode)
{
	uint32_t count = keyboard_buffer_item_count();
	if (count == _kbd_buffer_len - 1) {
		klogc(swarn, "Keyboard buffer is full!\n");
		return;
	}

	_kbd_buffer[_kbd_buffer_in++] = scancode;
	_kbd_buffer_in %= _kbd_buffer_len;
}

////////////////////////////////////////////////////////////////////////////////

uint8_t keyboard_read_scancode(void)
{
	while (!keyboard_has_items())
		thread_wait_keyboard();

	uint8_t scancode = _kbd_buffer[_kbd_buffer_out++];
	_kbd_buffer_out %= _kbd_buffer_len;
	return scancode;
}
