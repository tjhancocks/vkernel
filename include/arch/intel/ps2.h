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

#if !defined(PS2_H) && (__i386__ || __x86_64__)
#define PS2_H

union ps2_configuration
{
	struct {
		uint8_t port1_interrupt:1;
		uint8_t port2_interrupt:1;
		uint8_t system_flag:1;
		uint8_t zero1:1;
		uint8_t port1_clock:1;
		uint8_t port2_clock:1;
		uint8_t port_translation:1;
		uint8_t zero2:1;
	} __attribute__((packed)) s;
	uint8_t value;
};

union ps2_status
{
	struct {
		uint8_t output_buffer_status:1;
		uint8_t input_buffer_status:1;
		uint8_t system_flag:1;
		uint8_t command:1;
		uint8_t unknown:2;
		uint8_t timeout:1;
		uint8_t parity:1;
	} __attribute__((packed)) s;
	uint8_t value;
};

union ps2_controller_output
{
	struct {
		uint8_t system_reset:1; /* warning, do not set to zero! */
		uint8_t a20:1;
		uint8_t port2_clock:1;
		uint8_t port2_data:1;
		uint8_t port1_output_buffer_full:1;
		uint8_t port2_output_buffer_full:1;
		uint8_t port1_clock:1;
		uint8_t port1_data:1;
	} __attribute__((packed)) s;
	uint8_t value;
};

enum ps2_io_port
{
	ps2_data_port = 0x60, /* read/write */
	ps2_status_reg = 0x64, /* read */
	ps2_command_reg = 0x64, /* write */
};

enum ps2_controller_response
{
	ps2_response_ack = 0xFA,
	ps2_response_resend = 0xFE,
};

enum ps2_controller_command
{
	/* Command Flags */
	ps2_has_response                   = 0x100,
	ps2_has_data                       = 0x200,

	/* PS/2 Controller Commands */
	ps2_read_ram                       = 0x20 | ps2_has_response,
	ps2_write_ram                      = 0x60 | ps2_has_data,
	ps2_port2_disable                  = 0xA7,
	ps2_port2_enable                   = 0xA8,
	ps2_port2_test                     = 0xA9 | ps2_has_response,
	ps2_test                           = 0xAA | ps2_has_response,
	ps2_port1_test                     = 0xAB | ps2_has_response,
	ps2_port1_disable                  = 0xAD,
	ps2_port1_enable                   = 0xAE,
	ps2_read_input_port                = 0xC0,
	ps2_input_low_to_status            = 0xC1,
	ps2_input_hi_to_status             = 0xC2,
	ps2_read_output_port               = 0xD0 | ps2_has_response,
	ps2_next_to_output_port            = 0xD1,
	ps2_next_to_port1_output           = 0xD2,
	ps2_next_to_port2_output           = 0xD3,
	ps2_next_to_port2_input            = 0xD4,
	ps2_pulse_output                   = 0xF0,
};

enum ps2_device_command
{
	/* Command Flags */
	ps2_device_has_ack                 = 0x100, /* ACK */
	ps2_device_has_first_byte          = 0x200, /* First response byte */
	ps2_device_has_second_byte         = 0x400, /* Second response byte */

	/* PS/2 Device Universal Commands */
	ps2_device_enable_scan             = 0xF4 | ps2_device_has_ack,
	ps2_device_disable_scan            = 0xF5 | ps2_device_has_ack,
	ps2_device_identify                = 0xF2 | ps2_device_has_ack
	                                          | ps2_device_has_first_byte
	                                          | ps2_device_has_second_byte,
	ps2_device_reset                   = 0xFF | ps2_device_has_ack
	                                          | ps2_device_has_first_byte
	                                          | ps2_device_has_second_byte,
};

enum ps2_device_type
{
	ps2_device_unknown = 0xFFFE, /* Unknown Device */
	ps2_device_at_keyboard = 0xFFFF, /* Invalid response */
	ps2_device_mouse = 0x0000,
	ps2_device_mouse_scroll_wheel = 0x0300,
	ps2_device_mouse_5btn = 0x0400,
	ps2_device_mf2_keyboard_translation_1 = 0x41AB,
	ps2_device_mf2_keyboard_translation_2 = 0xC1AB,
	ps2_device_mf2_keyboard = 0x83AB,
};

void init_ps2_controller(void);

#endif