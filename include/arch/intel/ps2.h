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

enum ps2_port
{
	ps2_cmd_port = 0x64,
	ps2_status_port = 0x64,
	ps2_data_port = 0x60,
};

enum ps2_status
{
	ps2_status_output = (1 << 0),
	ps2_status_input = (1 << 1),
	ps2_status_system = (1 << 2),
	ps2_status_command = (1 << 3),
	ps2_status_timeout = (1 << 6),
	ps2_status_parity_error = (1 << 7),
};

enum ps2_cmd
{
	/* Returns the controller configuration byte as a response. */
	ps2_cmd_read_cfg = 0x20,

	/* Test the PS/2 controller. Returns either 0x55 (PASS) or 0xFC (FAIL). */
	ps2_cmd_test = 0xAA,

	/* Disable/Enable second PS/2 port. */
	ps2_cmd_enable_p2 = 0xA8,
	ps2_cmd_disable_p2 = 0xA7,

	/* Test the second PS/2 port. Returns 0x00 if test passed, or 0x01 to 0x04
	   if the test failed. */
	ps2_cmd_test_p2 = 0xA9,

	/* Disable/Enable first PS/2 port. */
	ps2_cmd_enable_p1 = 0xAE,
	ps2_cmd_disable_p1 = 0xAD,

	/* Test the first PS/2 port. Returns 0x00 if test passed, or 0x01 to 0x04
	   if the test failed. */
	ps2_cmd_test_p1 = 0x0AB,
};

enum ps2_cmd_response
{
	/* General responses */
	ps2_cmd_ack = 0xFA,
	ps2_cmd_resend = 0xFE,

	/* Specific responses */
	ps2_cmd_test_pass = 0x55,
	ps2_cmd_test_fail = 0xFC,
	ps2_cmd_port_test_pass = 0x00,
	ps2_cmd_port_test_clk_lo = 0x01,
	ps2_cmd_port_test_clk_hi = 0x02,
	ps2_cmd_port_test_data_lo = 0x03,
	ps2_cmd_port_test_data_hi = 0x04,
};

void init_ps2_controller(void);

#endif