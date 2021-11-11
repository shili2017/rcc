# Toolchain
ARCH = riscv64-unknown-elf
CC = $(ARCH)-gcc
AS = $(ARCH)-gcc
LD = $(ARCH)-ld
GDB = gdb-multiarch
OBJDUMP = $(ARCH)-objdump
OBJCOPY = $(ARCH)-objcopy

# Board
SBI = rustsbi
BOOTLOADER = ../bootloader/$(SBI)-qemu.bin

# Directories
WORK_DIR = $(shell pwd)
USER_DIR = $(WORK_DIR)/../user
BUILD_DIR = $(WORK_DIR)/build
DST_DIR = $(BUILD_DIR)/$(ARCH)
$(shell mkdir -p $(DST_DIR))

# Source
SRC = $(wildcard src/*.c src/*.S src/*/*.c src/*/*.S)
OBJ = $(addprefix $(DST_DIR)/, $(addsuffix .o, $(basename $(SRC))))
LINKER_SCRIPT = $(WORK_DIR)/src/linker.ld

# Image (todo)
FS_IMG = $(USER_DIR)/fs.img

# Library (libc)
LIB_DIR = $(realpath ../libc)
LIB_BUILD_DIR = $(LIB_DIR)/build
LIB_NAME = kernelc
LIB = $(LIB_BUILD_DIR)/lib$(LIB_NAME).a

# EFS
EFS_DIR = $(realpath ../efs)
EFS_BUILD_DIR = $(EFS_DIR)/build
EFS_NAME = efs
EFS = $(EFS_BUILD_DIR)/lib$(EFS_NAME).a

# Flags
INCFLAGS = -I$(WORK_DIR)/include -I$(LIB_DIR)/include -I$(EFS_DIR)/include
COMMON_FLAGS = -fno-pic -mcmodel=medany
CFLAGS = $(COMMON_FLAGS) -static -O0 -MMD -Wall -ggdb $(INCFLAGS) \
         -fno-builtin -fno-stack-protector -ffreestanding -Wno-main
ASFLAGS = $(COMMON_FLAGS) -O0 -MMD $(INCFLAGS)
LDFLAGS = -melf64lriscv -T $(LINKER_SCRIPT) \
					-L$(LIB_BUILD_DIR) -l$(LIB_NAME) \
					-L$(EFS_BUILD_DIR) -l$(EFS_NAME) \
					-g

# Build target
KERNEL_NAME = rcc
KERNEL_ELF = $(BUILD_DIR)/$(KERNEL_NAME).elf
KERNEL_BIN = $(BUILD_DIR)/$(KERNEL_NAME).bin
KERNEL_DISASM = $(BUILD_DIR)/$(KERNEL_NAME).txt

# Kernel entry
KERNEL_ENTRY_PA = 0x80200000

# Kernel log level
LOG_LEVELS = ERROR WARN INFO DEBUG TRACE
ifeq ($(LOG),)
LOG = INFO
else
ifeq ($(filter $(LOG_LEVELS), $(LOG)), )
  $(error Expected $$LOG in {$(LOG_LEVELS)}, Got "$(LOG)")
endif
endif
CFLAGS += -DLOG_LEVEL=LOG_$(LOG)

default: build

build: build-lib build-user build-efs $(KERNEL_ELF)

build-lib:
	cd $(LIB_DIR) && $(MAKE) KERNEL=1

build-user:
	cd $(USER_DIR) && $(MAKE)

build-efs:
	cd $(EFS_DIR) && $(MAKE) KERNEL=1

$(KERNEL_ELF): $(OBJ)
	$(LD) $^ $(LIB) -o $@ $(LDFLAGS)
	$(OBJCOPY) $(KERNEL_ELF) --strip-all -O binary $(KERNEL_BIN)
	$(OBJDUMP) $(KERNEL_ELF) -d > $(KERNEL_DISASM)

# Compile: a single `.c` -> `.o` (gcc)
$(DST_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $(realpath $<)

# Compile: a single `.S` -> `.o` (gcc)
$(DST_DIR)/%.o: %.S
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -c -o $@ $(realpath $<)

run: build
	@qemu-system-riscv64 \
		-machine virt \
		-nographic \
		-bios $(BOOTLOADER) \
		-device loader,file=$(KERNEL_BIN),addr=$(KERNEL_ENTRY_PA) \
		-drive file=$(FS_IMG),if=none,format=raw,id=x0 \
        -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0

debug-run:
	@qemu-system-riscv64 \
		-machine virt \
		-nographic \
		-bios $(BOOTLOADER) \
		-device loader,file=$(KERNEL_BIN),addr=$(KERNEL_ENTRY_PA) \
		-drive file=$(FS_IMG),if=none,format=raw,id=x0 \
        -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0 \
		-s -S

debug-gdb:
	$(GDB) $(KERNEL_ELF) -ex 'target remote localhost:1234'

clean:
	-rm -rf $(BUILD_DIR)

clean-all: clean
	cd $(LIB_DIR) && $(MAKE) clean
	cd $(USER_DIR) && $(MAKE) clean
	cd $(EFS_DIR) && $(MAKE) clean

.PHONY: default build build-lib build-user run debug-run debug-gdb clean clean-all fs-img
