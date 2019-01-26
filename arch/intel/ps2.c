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
#include <time.h>
#include <keyboard.h>

////////////////////////////////////////////////////////////////////////////////

static struct
{
	bool enabled;
	bool port1_valid;
	enum ps2_device_type port1_device;
	bool port2_valid;
	enum ps2_device_type port2_device;
	bool dual_channel;
	union ps2_configuration configuration;
} ps2;

////////////////////////////////////////////////////////////////////////////////

static inline bool __ps2_can_write(void)
{
	union ps2_status status = { .value = inb(ps2_status_reg) };
	return status.s.input_buffer_status ? false : true;
}

static inline bool __ps2_can_read(void)
{
	union ps2_status status = { .value = inb(ps2_status_reg) };
	return status.s.output_buffer_status ? true : false;
}

static inline void __ps2_write_data(uint8_t data)
{
	while (!__ps2_can_write()) 
		nop();
	outb(ps2_data_port, data);
}

static inline uint8_t __ps2_read_data(void)
{
	while (!__ps2_can_read())
		nop();
	return inb(ps2_data_port);
}

static inline bool __ps2_read_data_timeout(uint8_t *data)
{
	uint64_t timeout = uptime_ms() + 50;
	while ((timeout >= uptime_ms()) && !__ps2_can_read())
		nop();

	if (__ps2_can_read()) {
		/* got the byte successfully! */
		*data = inb(ps2_data_port);
		return true;
	}

	/* timed out */
	return false;
}

static inline void __ps2_send_command(uint8_t cmd)
{
	/* todo: check if there needs to be a wait here */
	outb(ps2_command_reg, cmd);
}

////////////////////////////////////////////////////////////////////////////////

static bool ps2_controller_send(
	enum ps2_controller_command cmd, uint8_t data, uint8_t *response
) {
	for (uint8_t attempt = 0; attempt < 3; ++attempt) {
		uint8_t cmd_byte = cmd & 0xFF;
		bool send_data = cmd & ps2_has_data;
		bool expect_response = cmd & ps2_has_response;

		/* The command must be sent! */
		__ps2_send_command(cmd_byte);

		/* Are we expected to send data? A second byte? */
		if (send_data) {
			__ps2_write_data(data);
		}

		/* Are we expecting a response? */
		if (!expect_response) {
			return true;
		}
		
		/* Read the response. Are we required to resend the command? */
		if ((*response = __ps2_read_data()) != ps2_response_resend) {
			return true;
		}
	}

	/* Reaching this point is bad. We failed to send the command successfully */
	return false;
}

static bool ps2_device_send(
	enum ps2_device_command cmd, uint8_t port, uint8_t data, uint16_t *response
) {
	if (port < 1 || port > 2) 
		return false;

	for (uint8_t attempt = 0; attempt < 3; ++attempt) {
		uint8_t cmd_byte = cmd & 0xFF;
		bool has_ack = cmd & ps2_device_has_ack;
		bool has_first_byte = cmd & ps2_device_has_first_byte;
		bool has_second_byte = cmd & ps2_device_has_second_byte;

		/* If the command is intended for the second port, make sure it goes
		   there */
		if (port == 2) {
			ps2_controller_send(ps2_next_to_port2_input, 0, NULL);
		}

		/* Send the actual command */
		__ps2_write_data(cmd_byte);

		/* Are we waiting for an ACK? */
		if (has_ack) {
			uint8_t ack = __ps2_read_data();
			if (ack != 0xFA) {
				continue;
			}
		}

		/* todo: This next section _really_ should have a timeout */
		/* Wait for any response bytes from the device. */
		if (has_first_byte) {
			uint8_t byte;

			if (__ps2_read_data_timeout(&byte)) {
				*response |= byte;

				if (has_second_byte) {
					if (__ps2_read_data_timeout(&byte)) {
						*response |= byte << 8;
					}
				}
			}
			else {
				*response = 0xFFFF;
			}
		}

		return true;
	}

	/* if we reached this point then we failed */
	return false;
}

////////////////////////////////////////////////////////////////////////////////

static uint8_t ps2_keyboard_scan(uint8_t *scancode)
{
	return (*scancode = __ps2_read_data());
}

static void ps2_flush(void)
{
	while (__ps2_can_read())
		(void)__ps2_read_data();
}

////////////////////////////////////////////////////////////////////////////////

static void ps2_keyboard_irq_handler(uint8_t irq __attribute__((unused)))
{
	uint8_t scancode = 0;
	ps2_keyboard_scan(&scancode);
	keyboard_record_scancode(scancode);
}

static void ps2_mouse_irq_handler(uint8_t irq __attribute__((unused)))
{
	/* todo */
	ps2_flush();
}

////////////////////////////////////////////////////////////////////////////////

static void ps2_read_configuration(void)
{
	klogc(sinfo, "Reading PS/2 Configuration\n");
	ps2_controller_send(ps2_read_ram, 0, &ps2.configuration.value);
}

static void ps2_update_configuration(void)
{
	klogc(sinfo, "Writing PS/2 Configuration\n");
	ps2_controller_send(ps2_write_ram, ps2.configuration.value, NULL);
}

////////////////////////////////////////////////////////////////////////////////

static bool ps2_test_port(uint8_t port)
{
	if (port < 1 || port > 2) 
		return false;

	enum ps2_controller_command cmd = (
		port == 1 ? ps2_port1_test : ps2_port2_test
	);
	uint8_t response = 0;

	ps2_controller_send(cmd, 0, &response);
	switch (response) {
		case 0x00: /* Passed */
			klogc(sok, "PS/2 Port %d passed self test\n", port);
			return true;
		case 0x01: /* Clock line stuck low */
		case 0x02: /* Clock line stuck high */
		case 0x03: /* Data line stuck low */
		case 0x04: /* Data line stuck high */
		default:   /* Unknown result */
			klogc(sok, "PS/2 Port %d failed self test\n", port);
			return false;
	}
}

static bool ps2_port_enabled(uint8_t port, bool enable)
{
	if (port < 1 || port > 2) 
		return false;

	enum ps2_controller_command cmd = (
		enable ? (port == 1 ? ps2_port1_enable : ps2_port2_enable)
		       : (port == 1 ? ps2_port1_disable : ps2_port2_disable)
	);

	klogc(
		sinfo, "Setting PS/2 Port %d to %sabled\n", port, enable ? "en" : "dis"
	);
	ps2_controller_send(cmd, 0, NULL);

	return true;
}

static bool ps2_port_interrupts_enabled(uint8_t port, bool enable)
{
	if (port < 1 || port > 2) 
		return false;

	if (port == 1) {
		ps2.configuration.s.port1_interrupt = enable;
		ps2.configuration.s.port1_clock = !enable;
	}
	else if (port == 2) {
		ps2.configuration.s.port2_interrupt = enable;
		ps2.configuration.s.port2_clock = !enable;
	}

	klogc(
		sinfo, "%sabling PS/2 Port %d interrupts\n", enable ? "En" : "Dis", port
	);
	ps2_update_configuration();

	return true;
}

static inline bool ps2_device_is_keyboard(uint16_t type)
{
	switch (type) {
		case ps2_device_at_keyboard:
		case ps2_device_mf2_keyboard_translation_1:
		case ps2_device_mf2_keyboard_translation_2:
		case ps2_device_mf2_keyboard:
			return true;
		default: 
			return false;
	}
}

static inline bool ps2_device_is_mouse(uint16_t type)
{
	switch (type) {
		case ps2_device_mouse:
		case ps2_device_mouse_scroll_wheel:
		case ps2_device_mouse_5btn:
			return true;
		default: 
			return false;
	}
}

static bool ps2_port_detect_device(uint8_t port, enum ps2_device_type *type)
{
	if (port < 1 || port > 2) 
		return false;

	/* Query the device for its identity. */
	uint16_t response = 0;
	if (!ps2_device_send(ps2_device_disable_scan, port, 0, &response)) {
		return false;
	}

	if (!ps2_device_send(ps2_device_identify, port, 0, &response)) {
		return false;
	}

	/* We must re-enable scanning, otherwise nothing will be sent back from
	   the device when it is used. */
	uint16_t tmp = 0;
	if (!ps2_device_send(ps2_device_enable_scan, port, 0, &tmp)) {
		return false;
	}

	/* Lookup what the response indicates. */
	if (ps2_device_is_mouse(response)) {
		klogc(sok, "PS/2 Device on Port %d is a mouse\n", port);
	}
	else if (ps2_device_is_keyboard(response)) {
		klogc(sok, "PS/2 Device on Port %d is a keyboard\n", port);
	}
	else {
		klogc(swarn, "PS/2 Device on Port %d is unknown\n", port);
		response = ps2_device_unknown;
	}

	/* Pass the device type back to the caller for immediate use */
	*type = response;

	/* Was the operation successful? Also store the device type in the driver */
	if (port == 1) {
		return ((ps2.port1_device = response) != ps2_device_unknown);
	}
	else if (port == 2) {
		return ((ps2.port2_device = response) != ps2_device_unknown);
	}
}

static bool ps2_reset_device(uint8_t port)
{
	if (port < 1 || port > 2) 
		return false;

	/* Attempt to reset the device. */
	uint16_t response = 0;
	if (!ps2_device_send(ps2_device_reset, port, 0, &response)) {
		return false;
	}

	switch (response) {
		case 0x00AA: /* Success */
			return true;
		default:
			return false;
	}
}

static bool ps2_configure_port(uint8_t port)
{
	if (port < 1 || port > 2) 
		return false;

	/* Make sure the port itself is enabled, as we need to talk to the device */
	if (!ps2_port_enabled(port, true)) {
		return false;
	}

	/* Ensure its interrupt line is enabled. */
	ps2_port_interrupts_enabled(port, true);

	/* Send a reset request to the device */
	if (!ps2_reset_device(port)) {
		klogc(swarn, "PS/2 Device on Port %d failed to reset!\n", port);
		return false;
	}

	/* Ask the device what it is. */
	enum ps2_device_type device_type = ps2_device_unknown;
	if (!ps2_port_detect_device(port, &device_type)) {
		return false;
	}

	/* Bind an appropriate IRQ handler to it. */
	uint8_t irq = (port == 1 ? 0x21 : 0x2C);
	if (ps2_device_is_keyboard(device_type)) {
		klog("Installing PS/2 Keyboard Handler for IRQ %02x\n", irq);
		set_irq_handler(irq, ps2_keyboard_irq_handler);
	}
	else if (ps2_device_is_mouse(device_type)) {
		klog("Installing PS/2 Mouse Handler for IRQ %02x\n", irq);
		set_irq_handler(irq, ps2_mouse_irq_handler);
	}

	ps2_flush();

	klogc(sok, "PS/2 Port %d is enabled.\n", port);
	return true;
}

////////////////////////////////////////////////////////////////////////////////

static bool ps2_self_test(void)
{
	uint8_t response = 0;
	ps2_controller_send(ps2_test, 0, &response);

	switch (response) {
		case 0x55: /* Passed */
			klogc(sok, "PS/2 controller passed self test\n");
			return true;
		case 0xFC: /* Failed */
		default:   /* Unknown result */
			klogc(sok, "PS/2 controller failed self test\n");
			return false;
	}
}

////////////////////////////////////////////////////////////////////////////////

void init_ps2_controller(void)
{
	klogc(sinfo, "Initialising PS/2 Controller\n");

	/* Ensure both PS/2 ports are disabled. If we're dealing with single channel
	   controller, then the second disable command will be ignored. */
	ps2_port_enabled(1, false);
	ps2_port_enabled(2, false);

	/* Get rid of any residual data in the buffers */
	ps2_flush();

	/* Read the configuration of the PS/2 controller. Make sure everything is
	   valid. To this make sure bits 0, 1 and 6 are clear. We can also check
	   bit 5 to see if there is a second PS/2 port. */
	/* TODO: Bit 6 is currently being left intact due to translation being 
	   required currently. This needs to be fixed in the future! */
	ps2_read_configuration();
	ps2.dual_channel = ps2.configuration.value & 0x05;
	ps2.configuration.value &= ~(0x03);
	ps2_update_configuration();

	klog(
		"PS/2 Controller is %s channel.\n", ps2.dual_channel ? "dual" : "single"
	);

	/* Perform the required tests of both the controller and the ports. */
	if (!(ps2.enabled = ps2_self_test())) {
		klogc(serr, "PS/2 Controller failed self test. Disabling PS/2.\n");
		return;
	}

	if (!(ps2.port1_valid = ps2_test_port(1))) {
		klogc(serr, "PS/2 Port 1 failed self test. Disabling Port 1.\n");
		return;
	}

	if (ps2.dual_channel) {
		if (!(ps2.port2_valid = ps2_test_port(2))) {
			klogc(serr, "PS/2 Port 2 failed self test. Disabling Port 2.\n");
			return;
		}
	}

	/* Enable the ports, after checking if they should in fact be enabled. */
	if (ps2.port1_valid) {
		if (!ps2_configure_port(1)) {
			ps2.port1_valid = false;
			ps2.port1_device = ps2_device_unknown;
		}
	}

	if (ps2.dual_channel && ps2.port2_valid) {
		if (!ps2_configure_port(2)) {
			ps2.port2_valid = false;
			ps2.port2_device = ps2_device_unknown;
		}
	}

	/* The PS/2 Controller is finished being initialised. Make sure it is marked
	   as active */
	ps2.enabled = true;
}

#endif