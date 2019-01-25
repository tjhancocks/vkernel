# Copyright (c) 2018-2019 Tom Hancocks
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

################################################################################
# DEFINITIONS

ROOT = $(CURDIR)
BUILD = $(ROOT)/build
TARGET.TRIPLET = i686-elf
TARGET.install-path = $(shell if [[ ! -z "${INSTALL_PATH}" ]]; then \
	echo "${INSTALL_PATH}/vkernel"; \
else \
	echo "/tmp/vkernel"; \
fi)

uniq = $(if $1,$(firstword $1) $(call uniq,$(filter-out $(firstword $1),$1)))

source-files = $(shell find $(1) -type f \( -name "*.$(2)" \))

build-files-from-source = $(addprefix $(BUILD)/, \
	$(addsuffix .$(2), $(basename $(1:$(ROOT)/%=%)))\
)

build-files = $(call \
	build-files-from-source,$(call source-files,$(1),$(2)),$(3)\
)

OBJECTS.c = $(call build-files,$(ROOT),c,o)
OBJECTS.s = $(call build-files,$(ROOT),s,s.o)
OBJECTS   = $(OBJECTS.c) $(OBJECTS.s)

KERNEL.binary = $(BUILD)/kernel
KERNEL.ramdisk = $(BUILD)/dsysrd

RAMDISK.files = ramdisk/contents
TOOL.TAR.flags = c -f $(KERNEL.ramdisk) $(RAMDISK.files)

TOOL.CC.flags = -ffreestanding -Wall -Wextra -nostdlib -nostdinc -fno-builtin\
	-fno-stack-protector -nostartfiles -nodefaultlibs -m32\
	-finline-functions -std=c11 -O0 -fstrength-reduce\
	-fomit-frame-pointer -c -I./include -DUSE_SERIAL\
	-D__KERNEL_NAME__="\"vkernel\"" -D__KERNEL_VERSION__="\"0.1\""\
	-D__KERNEL_COMMIT__="\"n/a\""
TOOL.AS.flags = -felf
TOOL.LD.flags = -nostdlib -nostartfiles -L$(BUILD)

################################################################################
# TOOLING

TOOL.CC = $(shell which $(TARGET.TRIPLET)-gcc)
LIBGCC = $(shell $(TOOL.CC) -print-libgcc-file-name)

TOOL.LD = $(shell which $(TARGET.TRIPLET)-ld)
TOOL.AS = $(shell which nasm)
TOOL.TAR = $(shell which tar)

TOOL.Q = $(shell which qemu-system-x86_64)


################################################################################
# HIGH LEVEL RULES

.PHONY: all
all: kernel

.PHONY: clean
clean:
	-rm -rf $(BUILD)

.PHONY: kernel
kernel: $(KERNEL.binary)

.PHONY: run-q
run-q: clean kernel $(KERNEL.ramdisk)
	$(TOOL.Q) -kernel $(KERNEL.binary) -serial stdio -initrd $(KERNEL.ramdisk) \
		-soundhw pcspk

.PHONY: install
install: clean kernel
	-cp $(KERNEL.binary) $(TARGET.install-path)


################################################################################
# BUILD RULES

$(KERNEL.binary): $(OBJECTS)
	$(TOOL.LD) -Tlink.ld $(TOOL.LD.flags) -o $@ $^ $(LIBGCC)

$(OBJECTS.c):
	$(eval SRC := $(@:$(BUILD)%.o=$(ROOT)%.c))
	-mkdir -p $(*D)
	$(TOOL.CC) $(TOOL.CC.flags) -o $@ $(SRC)

$(OBJECTS.s):
	$(eval SRC := $(@:$(BUILD)%.o=$(ROOT)%))
	-mkdir -p $(*D)
	$(TOOL.AS) $(TOOL.AS.flags) -o $@ $(SRC)

$(KERNEL.ramdisk):
	$(TOOL.TAR) $(TOOL.TAR.flags)
