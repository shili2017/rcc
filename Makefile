# Toolchain
ARCH = riscv64-linux-gnu
CC = $(ARCH)-gcc
AS = $(ARCH)-gcc
LD = $(ARCH)-ld
GDB = $(ARCH)-gdb
OBJDUMP = $(ARCH)-objdump
OBJCOPY = $(ARCH)-objcopy

# Board
SBI = rustsbi
BOOTLOADER = bootloader/$(SBI)-qemu.bin

# Directories
WORK_DIR = $(shell pwd)
BUILD_DIR = $(WORK_DIR)/build
DST_DIR = $(WORK_DIR)/build/$(ARCH)
$(shell mkdir -p $(DST_DIR))

# Source
SRC = $(wildcard src/*.c src/*.S)
OBJ = $(addprefix $(DST_DIR)/, $(addsuffix .o, $(basename $(SRC))))
LINKER_SCRIPT = $(WORK_DIR)/src/linker.ld

# Flags
INCFLAGS = -I$(WORK_DIR)/include
COMMON_FLAGS = -fno-pic -mcmodel=medany
CFLAGS = $(COMMON_FLAGS) -static -O2 -MMD -Wall -ggdb $(INCFLAGS) \
         -fno-builtin -fno-stack-protector -ffreestanding -Wno-main
ASFLAGS = $(COMMON_FLAGS) -O0 -MMD $(INCFLAGS)
LDFLAGS = -melf64lriscv -T $(LINKER_SCRIPT) -g

# Build target
KERNEL_ELF = $(BUILD_DIR)/os.elf
KERNEL_BIN = $(BUILD_DIR)/os.bin
KERNEL_DISASM = $(BUILD_DIR)/os.txt

# Kernel entry
KERNEL_ENTRY_PA = 0x80200000

default: build

build: $(KERNEL_ELF)

$(KERNEL_ELF): $(OBJ)
	$(LD) $(LDFLAGS) $^ -o $@
	$(OBJCOPY) $(KERNEL_ELF) --strip-all -O binary $(KERNEL_BIN)
	$(OBJDUMP) $(KERNEL_ELF) -d > $(KERNEL_DISASM)

# Compile: a single `.c` -> `.o` (gcc)
$(DST_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c -o $@ $(realpath $<)

# Compile: a single `.S` -> `.o` (gcc)
$(DST_DIR)/%.o: %.S
	@mkdir -p $(dir $@)
	@$(AS) $(ASFLAGS) -c -o $@ $(realpath $<)

run: build
	@qemu-system-riscv64 \
		-machine virt \
		-nographic \
		-bios $(BOOTLOADER) \
		-device loader,file=$(KERNEL_BIN),addr=$(KERNEL_ENTRY_PA)

debug-run: build
	@qemu-system-riscv64 \
		-machine virt \
		-nographic \
		-bios $(BOOTLOADER) \
		-device loader,file=$(KERNEL_BIN),addr=$(KERNEL_ENTRY_PA) \
		-s -S

debug-gdb: build
	@$(GDB) $(KERNEL_ELF)

clean:
	-rm -rf $(BUILD_DIR)

.PHONY: default build run debug-run debug-gdb clean
