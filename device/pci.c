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

#include <pci.h>
#include <panic.h>
#include <print.h>

#define PCI_BUS_MAX    8
#define PCI_SLOT_MAX   32
#define PCI_FUNC_MAX   8

////////////////////////////////////////////////////////////////////////////////

static inline uint8_t pci_bus(uint32_t dev)
{
	return (dev >> 16) & 0xFF;
}

static inline uint8_t pci_slot(uint32_t dev)
{
	return (dev >> 8) & 0xFF;
}

static inline uint8_t pci_func(uint32_t dev)
{
	return dev & 0xFF;
}

static inline uint32_t pci_device(uint16_t bus, uint8_t slot, uint8_t func)
{
	return (bus << 16) | (slot << 8) | func;
}

static inline uint32_t pci_address(uint32_t dev, enum pci_field field)
{
	return (
		0x80000000 |
		(pci_bus(dev) << 16) |
		(pci_slot(dev) << 11) |
		(pci_func(dev) << 8) |
		(field & 0xFC)
	);
}

static uint32_t pci_read(uint32_t dev, enum pci_field field, uint8_t size)
{
	outl(pci_address_port, pci_address(dev, field));
	switch (size) {
		case  4: return inl(pci_data_port);
		case  2: return inl(pci_data_port);
		case  1:
		default: return inl(pci_data_port);
	}
}

static uint16_t pci_type(uint32_t dev)
{
	return (pci_read(dev, pci_field_class, 1) << 8) | 
		pci_read(dev, pci_field_subclass, 1);
}

////////////////////////////////////////////////////////////////////////////////

static void pci_scan_hit(
	pci_func_t f, uint32_t dev, void *info
) {
	uint16_t device_id = pci_read(dev, pci_field_device_id, 2);
	uint16_t vendor_id = pci_read(dev, pci_field_vendor_id, 2);

	if (device_id != 0xFFFF && vendor_id != 0xFFFF) {
		if (f) {
			f(dev, vendor_id, device_id, info);
		}
	}	
}

static void pci_scan_func(
	pci_func_t f, enum pci_type type, uint8_t bus, uint8_t slot, uint8_t func,
	void *info
) {
	uint32_t dev = pci_device(bus, slot, 0);
	if (type == -1 || type == pci_type(dev)) {
		pci_scan_hit(f, dev, info);
	}
	if (pci_type(dev) == pci_type_bridge) {
		/* Secondary PCI Bus */
		panic(
			"Unimplemented PCI Feature",
			"Secondary PCI Buses are currently unsupported."
		);
	}
}

static void pci_scan_slot(
	pci_func_t f, enum pci_type type, uint8_t bus, uint8_t slot, void *info
) {
	uint32_t dev = pci_device(bus, slot, 0);
	if (pci_read(dev, pci_field_vendor_id, 2) == pci_none) {
		return;
	}

	pci_scan_func(f, type, bus, slot, 0, info);
	if (!pci_read(dev, pci_field_header_type, 1)) {
		return;
	}

	for (uint8_t func = 1; func < PCI_FUNC_MAX; ++func) {
		uint32_t dev = pci_device(bus, slot, func);
		if (pci_read(dev, pci_field_vendor_id, 2) != pci_none) {
			pci_scan_func(f, type, bus, slot, func, info);
		}
	}
}

static void pci_scan_bus(
	pci_func_t f, enum pci_type type, uint8_t bus, void *info
) {
	for (uint8_t slot = 0; slot < PCI_SLOT_MAX; ++slot) {
		pci_scan_slot(f, type, bus, slot, info);
	}
}

void pci_check_buses(
	pci_func_t f, enum pci_type type, void *info
) {
	union pci_header header;
	header.value = pci_read(0, pci_field_header_type, 1);
	if (header.s.mf == 0) {
		/* Single PCI Host Controller */
		pci_scan_bus(f, type, 0, info);
	}
	else {
		/* Multiple PCI Host Controllers */
		panic(
			"Unimplemented PCI Feature",
			"Multiple PCI Host Controllers are currently unsupported."
		);
	}
}

////////////////////////////////////////////////////////////////////////////////

void init_pci(void)
{
	pci_check_buses(NULL, -1, NULL);
}