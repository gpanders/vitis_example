NAME := vitis_example
BOOTGEN := $(shell command -v bootgen 2>/dev/null)
XOBJS := $(patsubst %.c,%.xo,$(wildcard kernels/*/*.c))

# Directory to place output products
BUILD := build

# Directory to place XSCT workspace products
WORKSPACE := $(BUILD)/xsct

# Software emulation (sw_emu), hardware emulation (hw_emu), or hardware (hw)
TARGET := sw_emu

PLATFORM := $(BUILD)/platform/$(NAME).xpfm

.PHONY: all
all: host xclbin emconfig.json

host: $(wildcard src/host/*.cpp src/host/*.h)
	$(MAKE) -C src/host
	cp src/host/host $@

.PHONY: platform
platform: $(PLATFORM)

.PHONY: xclbin
xclbin: $(BUILD)/$(NAME).xclbin

.PHONY: kernels
kernels: $(XOBJS)

.PHONY: clean
clean:
	rm -rf *.log *.jou *.str _x .Xil _vimage

.PHONY: cleanall
cleanall: clean
	rm -rf kernels/*/*.xo $(BUILD)/$(NAME).xclbin $(PLATFORM)

.PHONY: cleanhost
cleanhost:
	$(MAKE) -C src/host clean

emconfig.json: $(PLATFORM)
	emconfigutil --platform $(PLATFORM) $@

$(PLATFORM): $(BUILD)/$(NAME).xsa $(BUILD)/sysroot
	xsct scripts/create_platform.tcl $< $(WORKSPACE) $(BUILD)

$(BUILD)/$(NAME).xclbin: config.ini $(XOBJS)
	v++ --platform $(PLATFORM) --config $< --link -t $(TARGET) -o $@ $(XOBJS)

kernels/%.xo: $(PLATFORM) kernels/%.c kernels/%.h
	v++ --platform $(PLATFORM) -t $(TARGET) -o $@ -c -I kernels/$(*D) -k $(*F) kernels/$*.c

$(BUILD)/$(NAME).xsa:
	vivado -mode tcl -source scripts/create_xsa.tcl -tclargs $(NAME)

$(BUILD)/boot/Image:
	$(error "Missing Linux kernel image at $@")

$(BUILD)/sysroot:
	$(error "Missing Linux sysroot at $@")

$(BUILD)/boot/fsbl.elf $(BUILD)/boot/pmufw.elf $(BUILD)/boot/u-boot.elf $(BUILD)/boot/bl31.elf:
	$(error "Missing $@")


