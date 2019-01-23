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

#if !defined(PCI_H)
#define PCI_H

#include <types.h>
#include <arch.h>

typedef void(*pci_func_t)();

enum pci_type
{
	pci_type_device,
	pci_type_bridge,
	pci_type_cardbus
};

////////////////////////////////////////////////////////////////////////////////

void init_pci(void);
void pci_check_buses(pci_func_t f, enum pci_type type, void *info);

////////////////////////////////////////////////////////////////////////////////

enum pci_port 
{ 
	pci_address_port = 0xcf8, 
	pci_data_port = 0xcfc 
};

enum pci_value
{
	pci_none = 0xFFFF
};

enum pci_field
{
	pci_field_vendor_id           = 0x00 /* 2 Bytes */,
	pci_field_device_id           = 0x02 /* 2 Bytes */,
	pci_field_command             = 0x04 /* 2 Bytes */,
	pci_field_status              = 0x06 /* 2 Bytes */,
	pci_field_revision            = 0x08 /* 2 Bytes */,
	pci_field_prog_if             = 0x09 /* 1 Byte  */,
	pci_field_subclass            = 0x0A /* 1 Byte  */,
	pci_field_class               = 0x0B /* 1 Byte  */,
	pci_field_cache_line_size     = 0x0C /* 1 Byte  */,
	pci_field_latency_timer       = 0x0D /* 1 Byte  */,
	pci_field_header_type         = 0x0E /* 1 Byte  */,
	pci_field_bist                = 0x0F /* 1 Byte  */,
	pci_field_bar0                = 0x10 /* 4 Bytes */,
	pci_field_bar1                = 0x14 /* 4 Bytes */,
	pci_field_bar2                = 0x18 /* 4 Bytes */,
	pci_field_bar3                = 0x1C /* 4 Bytes */,
	pci_field_bar4                = 0x20 /* 4 Bytes */,
	pci_field_bar5                = 0x24 /* 4 Bytes */,
	pci_field_cardbus             = 0x28 /* 4 Bytes */,
	pci_field_subsystem_vendor_id = 0x2C /* 2 Bytes */,
	pci_field_subsystem_id        = 0x2E /* 2 Bytes */,
	pci_field_expansion_rom_base  = 0x30 /* 1 Byte  */,
	pci_field_capabilities        = 0x34 /* 1 Byte  */,
	/* Reserved Range 0x35 - 0x3B */
	pci_field_interrupt_line      = 0x3C /* 1 Byte  */,
	pci_field_interrupt_pin       = 0x3D /* 1 Byte  */,
	pci_field_min_grant           = 0x3E /* 1 Byte  */,
	pci_field_max_latency         = 0x3F /* 1 Byte  */
};

union pci_header
{
	uint8_t value;
	struct {
		uint8_t type:7;
		uint8_t mf:1;
	} s;
};

union pci_bist
{
	uint8_t value;
	struct {
		uint8_t code:4;
		uint8_t reserved:2;
		uint8_t start:1;
		uint8_t capable:1;
	} s;
};

union pci_command
{
	uint16_t value;
	struct {
		uint16_t io_space:1;
		uint16_t memory_space:1;
		uint16_t bus_master:1;
		uint16_t special_cycles:1;
		uint16_t memory_write:1;
		uint16_t vga_palette_snoop:1;
		uint16_t parity_error_response:1;
		uint16_t reserved0:1;
		uint16_t serr_enable:1;
		uint16_t fast_back_to_back:1;
		uint16_t interrupt_disable:1;
		uint16_t reserved1:4;
	} s;
};

union pci_status
{
	uint16_t value;
	struct {
		uint16_t reserved0:2;
		uint16_t interrupt:1;
		uint16_t capabilities_list:1;
		uint16_t is_66Mhz:1;
		uint16_t reserved1:1;
		uint16_t fast_back_to_back:1;
		uint16_t master_data_parity_error:1;
		uint16_t devsel_timing:2;
		uint16_t signaled_target_abort:1;
		uint16_t received_target_abort:1;
		uint16_t received_master_abort:1;
		uint16_t signaled_system_error:1;
		uint16_t detected_parity_error:1;
	} s;
};

union pci_bar
{
	uint32_t value;
	struct {
		uint32_t zero:1;
		uint32_t type:1;
		uint32_t prefetch:1;
		uint32_t address:28;
	} master;
	struct {
		uint32_t one:1;
		uint32_t reserved:1;
		uint32_t address:30;
	} io;
};

enum pci_class 
{
	pci_class_unclassified = 0x00,
	pci_class_mass_storage_controller = 0x01,
	pci_class_network_controller = 0x02,
	pci_class_display_controller = 0x03,
	pci_class_multimedia_controller = 0x04,
	pci_class_memory_controller = 0x05,
	pci_class_brige_device = 0x06,
	pci_class_simple_com_controller = 0x07,
	pci_class_base_peripheral = 0x08,
	pci_class_input_device_controller = 0x09,
	pci_class_docking_station = 0x0A,
	pci_class_processor = 0x0B,
	pci_class_serial_bus_controller = 0x0C,
	pci_class_wireless_controller = 0x0D,
	pci_class_intelligent_controller = 0x0E,
	pci_class_satellite_com_controller = 0x0F,
	pci_class_encryption_controller = 0x10,
	pci_class_signal_processing_controllerr = 0x11,
	pci_class_processing_accelerator = 0x12,
	pci_class_non_essential_instrumentation = 0x13,
	pci_class_coprocessor = 0x40,
	pci_class_unassigned = 0xFF, 
};

static const char *_pci_class_string[] = {
	"Unclassified PCI Device",
	"Mass Storage Controller",
	"Network Controller",
	"Display Controller",
	"Multimedia Controller",
	"Memory Controller",
	"Bridge Device",
	"Simple Communication Controller",
	"Base System Peripheral",
	"Input Device Controller",
	"Docking Station",
	"Processor",
	"Serial Bus Controller",
	"Wireless Controller",
	"Intelligent Controller",
	"Satellite Communication Controller",
	"Encryption Controller",
	"Signal Processing Controller",
	"Processing Accelerator",
	"Non-essential Instrumentation",
	"Coprocessor",
	"Unassigned PCI Class",
};

enum pci_subclass
{
	/* Class: Unclassified */
	pci_subclass_non_vga_device = 0x00,
	pci_subclass_vga_device = 0x01,

	/* Class: Mass Storage Controller */
	pci_subclass_scsi_bus_controller = 0x00,
	pci_subclass_ide_controller = 0x01,
	pci_subclass_floppy_disk_controller = 0x02,
	pci_subclass_ipi_bus_controller = 0x03,
	pci_subclass_raid_controller = 0x04,
	pci_subclass_ata_controller = 0x05,
	pci_subclass_serial_ata = 0x06,
	pci_subclass_serial_attached_scsi = 0x07,
	pci_subclass_non_volatile_memory_controller = 0x08,
	pci_subclass_other = 0x80,

	/* Class: Network Controller */
	pci_subclass_ethernet_controller = 0x00,
	pci_subclass_token_ring_controller = 0x01,
	pci_subclass_fddi_controller = 0x02,
	pci_subclass_atm_controller = 0x03,
	pci_subclass_isdn_controller = 0x04,
	pci_subclass_wordfip_controller = 0x05,
	pci_subclass_picmg_multi_computing = 0x06,
	pci_subclass_infiniband_controller = 0x07,
	pci_subclass_fabric_controller = 0x08,

	/* Class: Display Controller */
	pci_subclass_vga_controller = 0x00,
	pci_subclass_xga_controller = 0x01,
	pci_subclass_3D_controller = 0x02,

	/* Class: Multimedia Controller */
	pci_subclass_video_controller = 0x00,
	pci_subclass_audio_controller = 0x01,
	pci_subclass_telephony_device = 0x02,
	pci_subclass_audio_device = 0x03,

	/* Class: Memory Controller */
	pci_subclass_ram_controller = 0x00,
	pci_subclass_flash_controller = 0x01,

	/* Class: Bridge Device */
	pci_subclass_host_bridge = 0x00,
	pci_subclass_isa_bridge = 0x01,
	pci_subclass_eisa_bridge = 0x02,
	pci_subclass_mca_bridge = 0x03,
	pci_subclass_pci_bridge = 0x04,
	pci_subclass_pcmcia_bridge = 0x05,
	pci_subclass_nubus_bridge = 0x06,
	pci_subclass_cardbus_bridge = 0x07,
	pci_subclass_raceway_bridge = 0x08,
	pci_subclass_alt_pci_bridge = 0x09,
	pci_subclass_infiniband_bridge = 0x0A,

	/* Class: Simple Communication Controller */
	pci_subclass_serial_controller = 0x00,
	pci_subclass_parallel_controller = 0x01,
	pci_subclass_multiport_serial_controller = 0x02,
	pci_subclass_modem = 0x03,
	pci_subclass_gpib_controller = 0x04,
	pci_subclass_smartcard = 0x05,

	/* Class: Base System Peripheral */
	pci_subclass_pic = 0x00,
	pci_subclass_dma_controller = 0x01,
	pci_subclass_timer = 0x02,
	pci_subclass_rtc_controller = 0x03,
	pci_subclass_pci_hot_plug_controller = 0x04,
	pci_subclass_sd_host_controller = 0x05,
	pci_subclass_iommu = 0x06,

	/* Class: Input Device Controller */
	pci_subclass_keyboard_controller = 0x00,
	pci_subclass_digitizer_pen = 0x01,
	pci_subclass_mouse_controller = 0x02,
	pci_subclass_scanner_controller = 0x03,
	pci_subclass_gameport_controller = 0x04,

	/* Class: Docking Station */
	pci_subclass_generic_docking_station = 0x00,

	/* Class: Processor */
	pci_subclass_i386 = 0x00,
	pci_subclass_i486 = 0x01,
	pci_subclass_pentium = 0x02,
	pci_subclass_alpha = 0x10,
	pci_subclass_powerpc = 0x20,
	pci_subclass_mips = 0x30,
	pci_subclass_coprocessor = 0x40,

	/* Class: Serial Bus Controller */
	pci_subclass_firewire_controller = 0x00,
	pci_subclass_access_bus = 0x01,
	pci_subclass_ssa = 0x02,
	pci_subclass_usb_controller = 0x03,
	pci_subclass_fibre_channel = 0x04,
	pci_subclass_smbus = 0x05,
	pci_subclass_infiniband = 0x06,
	pci_subclass_ipmi_interface = 0x07,
	pci_subclass_sercos_interface = 0x08,
	pci_subclass_canbus = 0x09,

	/* Class: Wireless Controller */
	pci_subclass_irda_compatible_controller = 0x00,
	pci_subclass_consumer_ir_controller = 0x01,
	pci_subclass_rf_controller = 0x10,
	pci_subclass_bluetooth_controller = 0x11,
	pci_subclass_ethernet_controller_802_1_a = 0x20,
	pci_subclass_ethernet_controller_802_1_b = 0x21,

	/* Class: Intelligent Controller */
	pci_subclass_l20 = 0x00,

	/* Class: Satellite Communication Controller */ 
	pci_subclass_satellite_tv_controller = 0x01,
	pci_subclass_satellite_audio_controller = 0x02,
	pci_subclass_satellite_voice_controller = 0x03,
	pci_subclass_satellite_data_controller = 0x04,

	/* Class: Encryption Controller */
	pci_subclass_network_encryption = 0x00,
	pci_subclass_entertainment_encryption = 0x10,
	pci_subclass_other_encryption = 0x80,

	/* Class: Signal Processing Controller */
	pci_subclass_dpio_modules = 0x00,
	pci_subclass_performance_counters = 0x01,
	pci_subclass_communication_sync = 0x02,
	pci_subclass_signal_processing_management = 0x03,
};

enum pci_prog_if
{
	/* Subclass: IDE Controller */
	pci_prog_if_isa_compatible_mode_controller = 0x00,
	pci_prog_if_pci_native_mode_controller = 0x05,
	pci_prog_if_isa_compatible_pci_native = 0x0A,
	pci_prog_if_pci_native_isa_compatible = 0x0F,
	pci_prog_if_isa_compatible_bus_mastering = 0x80,
	pci_prog_if_pci_native_bus_mastering = 0x85,
	pci_prog_if_isa_compatible_pci_native_bus_mastering = 0x8A,
	pci_prog_if_pci_native_isa_compatible_bus_mastering = 0x8F,

	/* Subclass: ATA Controller */
	pci_prog_if_single_dma = 0x20,
	pci_prog_if_chained_dma = 0x30,

	/* Subclass: Serial ATA */
	pci_prog_if_vendor_specific_interface = 0x00,
	pci_prog_if_ahci = 0x01,
	pci_prog_if_sata_serial_storage_bus = 0x02,

	/* Subclass: Serial Attached SCSI */
	pci_prog_if_sas = 0x00,
	pci_prog_if_sas_serial_storage_bus = 0x01,

	/* Subclass: Non Volatile Memory Controller */
	pci_prog_if_ncmhci = 0x01,
	pci_prog_if_nvm_express = 0x02,

	/* Subclass: VGA Compatible Controller */
	pci_prog_if_vga_controller = 0x00,
	pci_prog_if_8514_compatible_controller = 0x01,

	/* Subclass: PCI-to-PCI Bridge */
	pci_prog_if_normal_decode = 0x00,
	pci_prog_if_subtractive_decode = 0x01,

	/* Subclass: RACEway Bridge */
	pci_prog_if_transparent_mode = 0x00,
	pci_prog_if_endpoint_mode = 0x01,
	pci_prog_if_semi_transparent_primary_bus = 0x40,
	pci_prog_if_semi_transparent_secondary_bus = 0x80,

	/* Subclass: Serial Controller */
	pci_prog_if_8250_compatible = 0x00,
	pci_prog_if_16450_compatible = 0x01,
	pci_prog_if_16550_compatible = 0x02,
	pci_prog_if_16650_compatible = 0x03,
	pci_prog_if_16750_compatible = 0x04,
	pci_prog_if_16850_compatible = 0x05,
	pci_prog_if_16950_compatible = 0x06,

	/* Subclass: Parallel Controller */
	pci_prog_if_standard_parallel_port = 0x00,
	pci_prog_if_bi_directional_parallel_port = 0x01,
	pci_prog_if_ecp_compliant_parallel_port = 0x02,
	pci_prog_if_ieee_1284_controller = 0x03,
	pci_prog_if_ieee_1284_target_device = 0x04,

	/* Subclass: Modem */
	pci_prog_if_genetic_modem = 0x00,
	pci_prog_if_hayes_16450_compatible = 0x01,
	pci_prog_if_hayes_16550_compatible = 0x02,
	pci_prog_if_hayes_16650_compatible = 0x03,
	pci_prog_if_hayes_16750_compatible = 0x04,
	pci_prog_if_hayes_16850_compatible = 0x05,

	/* Subclass: PIC */
	pci_prog_if_generic_8259_compatible = 0x00,
	pci_prog_if_pic_isa_compatible = 0x01,
	pci_prog_if_pic_eisa_compatible = 0x02,
	pci_prog_if_io_apic_interrupt_controller = 0x10,
	pci_prog_if_iox_apic_interrupt_controller = 0x20,

	/* Subclass: DMA Controller */
	pci_prog_if_generic_8237_compatible = 0x00,
	pci_prog_if_dma_isa_compatible = 0x01,
	pci_prog_if_dma_eisa_compatible = 0x02,

	/* Subclass: Timer */
	pci_prog_if_generic_8254_compatible = 0x00,
	pci_prog_if_timer_isa_compatible = 0x01,
	pci_prog_if_timer_eisa_compatible = 0x02,
	pci_prog_if_hpet = 0x03,

	/* Subclass: RTC Controller */
	pci_prog_if_generic_rtc = 0x00,
	pci_prog_if_rtc_isa_compatible = 0x01,

	/* Subclass: Gameport Controller */
	pci_prog_if_generic_gameport = 0x00,
	pci_prog_if_extended_gameport = 0x01,

	/* Subclass: FireWire Controller */
	pci_prog_if_firewire_generic = 0x00,
	pci_prog_if_firewire_ohci = 0x01,

	/* Subclass: USB Controller */
	pci_prog_if_usb_uhci_controller = 0x00,
	pci_prog_if_usb_ohci_controller = 0x01,
	pci_prog_if_usb_ehci_controller = 0x02,
	pci_prog_if_usb_xhci_controller = 0x03,
	pci_prog_if_usb_unspecified = 0x04,
	pci_prog_if_usb_device = 0xfe,

	/* Subclass: IPMI Interface */
	pci_prog_if_ipmi_smic = 0x00,
	pci_prog_if_ipmi_keyboard_controller_style = 0x01,
	pci_prog_if_ipmi_block_transfer = 0x02,
};

#endif