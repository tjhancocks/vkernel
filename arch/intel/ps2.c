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
#include <print.h>
#include <panic.h>
#include <keyboard.h>

////////////////////////////////////////////////////////////////////////////////

static struct {
	uint8_t response;
	uint8_t enabled;
	uint8_t cfg;
	uint8_t port1;
	uint8_t port2;
} ps2;

////////////////////////////////////////////////////////////////////////////////

static inline void __ps2_write_wait(void)
{
	while (inb(ps2_status_port) & ps2_status_input)
		nop();
}

static inline void __ps2_read_wait(void)
{
	while ((inb(ps2_status_port) & ps2_status_output) == 0)
		nop();
}

////////////////////////////////////////////////////////////////////////////////

static inline void __ps2_write(uint8_t data)
{
	__ps2_write_wait();
	outb(ps2_data_port, data);
}

static inline uint8_t __ps2_read(void)
{
	__ps2_read_wait();
	uint8_t data = inb(ps2_data_port);
	return data;
}

////////////////////////////////////////////////////////////////////////////////

static void __ps2_do_cmd(uint8_t cmd, uint8_t *resp)
{
	for (uint8_t attempt = 0; attempt < 3; ++attempt) {
		/* Send the command */
		outb(ps2_cmd_port, cmd);

		/* If we do not need a response then simply break out and finish. */
		if (!resp) 
			break;

		/* Wait for the response and then read it. Check if we need to handle
		   the response. */
		if ((*resp = __ps2_read()) != ps2_cmd_resend)
			break;

	}
}

static inline void ps2_send_cmd(uint8_t cmd)
{
	__ps2_do_cmd(cmd, NULL);
}

static inline uint8_t ps2_send_query(uint8_t cmd)
{
	__ps2_do_cmd(cmd, &ps2.response);
	return ps2.response;
}

static inline void ps2_kbd_scan(uint8_t *scan_code)
{
	ps2.response = __ps2_read();
	if (scan_code)
		*scan_code = ps2.response;
}

static inline void ps2_flush(void)
{
	while (inb(ps2_status_port) & ps2_status_output) {
		(void)inb(ps2_data_port);
	}
}

////////////////////////////////////////////////////////////////////////////////

static inline uint8_t ps2_port_test(uint8_t port)
{
	if (port < 1 || port > 2)
		return 0;

	switch (ps2_send_query(port == 1 ? ps2_cmd_test_p1 : ps2_cmd_test_p2)) {
		case ps2_cmd_port_test_pass: return 1;
		default: return 0;
	}
}

////////////////////////////////////////////////////////////////////////////////

static void ps2_kbd_interrupt_handler(void)
{
	uint8_t scan_code = 0;
	ps2_kbd_scan(&scan_code);

	/* TODO: Correctly handle the keyboard input. This should be directed to a
	   more generic keyboard driver that can handle PS/2 or USB keyboards. */
	keyboard_record_scancode(scan_code);
}

static void ps2_mouse_interrupt_handler(void)
{
	/* TODO: Handle mouse, and direct input to a generic mouse driver that can
	   handle all types of mice added to the system. */
}

static void ps2_irq_handler(uint8_t irq)
{
	/* TODO: This should use information about the device plugged in to the
	   port to determine which action to take. For now, we assume the correct
	   device is plugged into each port. */
	if (irq == 1) {
		ps2_kbd_interrupt_handler();
	}
	else if (irq == 12) {
		ps2_mouse_interrupt_handler();
	}
	else {
		klogc(serr, "*** Unexpected IRQ raising PS2 handler (%d)\n", irq);
	}
}

////////////////////////////////////////////////////////////////////////////////

void init_ps2_controller(void)
{
	/* TODO: Check for the presence of PS/2 controller and correctly initialise
	   USB (in order to disable legacy support) so that we are in a known good
	   state. Currently, we are making the assumption that this is all already
	   good. */

	/* Attempt to disable both PS/2 ports. */
	ps2_send_cmd(ps2_cmd_disable_p1);
	ps2_send_cmd(ps2_cmd_disable_p2);

	/* Flush out any residual data in the buffers */
	ps2_flush();

	/* Perform a configuration check of the PS/2 controller. */
	ps2.cfg = ps2_send_query(ps2_cmd_read_cfg);
	if (ps2.cfg & 0x02 == 0) {
		panic(
			"PS/2 Controller Reported POST Failure",
			"The PS/2 Controller has reported that the system failed to "
			"correctly POST. This should be impossible, and should mean that "
			"the system did not boot."
		);
	}

	/* Perform a check on the PS/2 Controller and its ports to make sure they
	   are functioning correctly. */
	switch (ps2_send_query(ps2_cmd_test)) {
		case ps2_cmd_test_pass: // Pass
			ps2.enabled = 1;
			break;

		case ps2_cmd_test_fail:
		default:
			klogc(serr, "*** Failed to initialise PS/2.\n");
			ps2.enabled = 0;
			break;
	}

	/* Perform a test on the first and second PS/2 port to ensure the ports are 
	   operating correctly. */
	ps2.port1 = ps2_port_test(1);
	ps2.port2 = ps2_port_test(2);

	/* Install the appropriate IRQ's to handle the PS/2 interrupts and enable 
	   the PS/2 ports that are correctly functionality/present. */
	if (ps2.port1) {
		klog("Enabling PS/2 Port 1\n");
		set_irq_handler(0x21, ps2_irq_handler);
		ps2_send_cmd(ps2_cmd_enable_p1);
	}

	if (ps2.port2) {
		klog("Enabling PS/2 Port 2\n");
		set_irq_handler(0x2C, ps2_irq_handler);
		ps2_send_cmd(ps2_cmd_enable_p2);
	}

	/* Finished initialising the PS/2 controller. */
}

#endif