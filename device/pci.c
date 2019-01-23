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
#include <print.h>

#define PCI_BUS_MAX		8
#define PCI_SLOT_MAX	32
#define PCI_FUNC_MAX	8

enum pci_port { pci_address_port = 0xcf8, pci_data_port = 0xcfc };
enum pci_field 
{
	pci_prog_if = 0x09, 
	pci_subclass = 0x0a, pci_class = 0x0b,
	pci_cache_line_size = 0x0c, pci_latency_timer = 0x0d,
	pci_header_type = 0x0e,
	pci_bist = 0x0f,
	pci_bar0 = 0x10, pci_bar1 = 0x14, pci_bar2 = 0x18, pci_bar3 = 0x1C,
	pci_bar4 = 0x20, pci_bar5 = 0x24,
	pci_secondary_bus = 0x19,
	pci_interrupt_line = 0x3C
};
enum pci_property
{
	pci_vendor_id = 0x00, pci_device_id = 0x02, 
	pci_cmd = 0x04, pci_status = 0x06, pci_rev_id = 0x08
};
enum pci_type
{
	pci_type_device = 0,
	pci_type_bridge = 1,
	pci_type_cardbus = 2
};
enum pci_value
{
	pci_none = 0xffff,
};

typedef void(*pci_func_t)(uint32_t, uint16_t, uint16_t, void *);

////////////////////////////////////////////////////////////////////////////////

static inline uint8_t pci_extract_bus(uint32_t dev)
{
	return (dev >> 16) & 0xFF;
}

static inline uint8_t pci_extract_slot(uint32_t dev)
{
	return (dev >> 8) & 0xFF;
}

static inline uint8_t pci_extract_fn(uint32_t dev)
{
	return dev & 0xFF;
}

static inline uint32_t pci_device(int32_t bus, uint32_t slot, uint32_t fn)
{
	return (uint32_t)((bus << 16) | (slot << 8) | fn);
}

static inline uint32_t pci_address(uint32_t dev, int32_t field)
{
	return (
		0x80000000 |
		(pci_extract_bus(dev) << 16) |
		(pci_extract_slot(dev) << 11) |
		(pci_extract_fn(dev) << 8) |
		(field & 0xFC)
	);
}

////////////////////////////////////////////////////////////////////////////////

static uint32_t pci_read(uint32_t dev, int32_t field, int32_t size)
{
	outl(pci_address_port, pci_address(dev, field));
	
	switch (size) {
		case 4:  return inl(pci_data_port);
		case 2:  return inw(pci_data_port);
		case 1:  return inb(pci_data_port);
		default: return 0xFFFF;
	}
}

////////////////////////////////////////////////////////////////////////////////

static void pci_scan_bus(pci_func_t, int32_t, int32_t, void*);
static void pci_scan_hit(pci_func_t, uint32_t, void*);
static void pci_scan_func(
	pci_func_t, int32_t, int32_t, int32_t, int32_t, void*
);
static void pci_scan_slot(pci_func_t, int32_t, int32_t, int32_t, void*);
static void pci_scan(pci_func_t, int32_t, void*);

////////////////////////////////////////////////////////////////////////////////

static inline uint16_t pci_find_type(uint32_t dev)
{
	return (pci_read(dev, pci_class, 1) << 8) | pci_read(dev, pci_subclass, 1);
}

static void pci_scan_hit(pci_func_t f, uint32_t dev, void *extra)
{
	uint16_t dev_vendor = pci_read(dev, pci_vendor_id, 2);
	uint16_t dev_id = pci_read(dev, pci_device_id, 2);

	if (f) {
		f(dev, dev_vendor, dev_id, extra);
	}

	klogc(
		sok, "Found PCI Device (vendor %04x, id %04x)\n", 
		dev_vendor, dev_id
	);
}

static void pci_scan_func(
	pci_func_t f, int32_t type, int32_t bus, int32_t slot, int32_t func, 
	void *extra
) {
	uint32_t dev = pci_device(bus, slot, 0);
	if (type == -1 || type == pci_find_type(dev)) {
		pci_scan_hit(f, dev, extra);
	}
	if (pci_find_type(dev) == pci_type_bridge) {
		pci_scan_bus(f, type, pci_read(dev, pci_secondary_bus, 1), extra);
	}
}

static void pci_scan_slot(
	pci_func_t f, int32_t type, int32_t bus, int32_t slot, void *extra
) {
	uint32_t dev = pci_device(bus, slot, 0);
	if (pci_read(dev, pci_vendor_id, 2) == pci_none) {
		return;
	}

	pci_scan_func(f, type, bus, slot, 0, extra);
	if (!pci_read(dev, pci_header_type, 1)) {
		return;
	}

	for (uint8_t func = 1; func < PCI_FUNC_MAX; ++func) {
		uint32_t dev = pci_device(bus, slot, func);
		if (pci_read(dev, pci_vendor_id, 2) != pci_none) {
			pci_scan_func(f, type, bus, slot, func, extra);
		}
	}
}

static void pci_scan_bus(pci_func_t f, int32_t type, int32_t bus, void *extra)
{
	for (uint8_t slot = 0; slot < PCI_SLOT_MAX; ++slot) {
		pci_scan_slot(f, type, bus, slot, extra);
	}
}

static void pci_scan(pci_func_t f, int32_t type, void *extra)
{
	if ((pci_read(0, pci_header_type, 1) & 0x80) == 0) {
		klogc(sinfo, "Performing restricted PCI scan\n");
		pci_scan_bus(f, type, 0, extra);
		return;
	}

	klogc(sinfo, "Performing full PCI scan\n");
}

////////////////////////////////////////////////////////////////////////////////

void init_pci(void)
{
	pci_scan(NULL, -1, NULL);
}