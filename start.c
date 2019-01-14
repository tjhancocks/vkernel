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

#include <types.h>
#include <arch.h>
#include <serial.h>
#include <print.h>
#include <pmm.h>
#include <multiboot.h>
#include <heap.h>
#include <display.h>
#include <ramdisk.h>
#include <context.h>
#include <panic.h>
#include <thread.h>

int thread_main(void)
{
	kprint("Hello, World!\a\n");
	return 0;
}

__attribute__((noreturn)) void kmain(void *mb, uint32_t boot_magic)
{
	/* Attempt to initialise serial port 1, if it is required/available. */
	init_serial(1);

	/* Ensure that the bootloader has done its job correctly. */
	struct multiboot_info *info = mb;
	if (boot_magic != MULTIBOOT_BOOTLOADER_MAGIC || info == NULL) {
		panic(
			"Multiboot Error", 
			"Invalid multiboot header [%08X].\n", boot_magic
		);
	}

	/* Initialise the hardware components of the system */
	init_physical_memory(mb);
	init_arch();
	init_display();

	/* Setup the kernel context. This will provide access to a heap and paging
	   functionality in the short term. */
	init_context(&kernel_context);

	/* Check if we have any modules to load in. We need to be sure that we have
	   got the system ramdisk initialised in order to proceed. */
	if (info->flags & MULTIBOOT_INFO_MODS) {
		if (info->mods_count > 0) {
			multiboot_module_t *mod = (void *)info->mods_addr;
			uint32_t mod_count = info->mods_count;

			for (uint32_t i = 0; i < mod_count; ++i) {
				/* TODO: Determine how to handle the particular module.
				   For now we're going to assume the module is the system 
				   ramdisk that needs to be loaded. */
				init_ramdisk(&system_ramdisk, mod[i].mod_start, mod[i].mod_end);
			}
		}
	}

	/* Ensure the system ramdisk is actually loaded. */
	if (!chk_ramdisk(&system_ramdisk)) {
		panic(
			"System Ramdisk Error",
			"Failed to locate the system ramdisk \"/sysrd\" and could not "
			"continue to boot. Please ensure that the system ramdisk file is "
			"in the correct location on the boot disk."
		);
	}

	/* Display the boot_message if it exists. */
	void *boot_message = ramdisk_open(&system_ramdisk, "bootmessage.txt", NULL);
	if (boot_message) {
		kprint(boot_message);
	}

	/* Setup threading and multitasking */
	init_threading();
	thread_create(thread_main);

	/* Enter an infinite loop to ensure we don't fall out of the kernel. We
	   should also perform some maintainence tasks periodically in here. */
	kprint("Kernel Main!\n");
	for (;;) {
		hang();
	}
}
