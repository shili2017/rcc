# Toolchain
ARCH = riscv64-unknown-elf
CC = $(ARCH)-gcc
AS = $(ARCH)-gcc
LD = $(ARCH)-ld
AR = $(ARCH)-ar

# Directories
WORK_DIR = $(shell pwd)
BUILD_DIR = $(WORK_DIR)/build
DST_DIR = $(WORK_DIR)/build/$(ARCH)
$(shell mkdir -p $(DST_DIR))

# Source
SRC = $(wildcard src/*.c src/*.S)
OBJ = $(addprefix $(DST_DIR)/, $(addsuffix .o, $(basename $(SRC))))

# Library (libc)
LIB_DIR = $(realpath ../libc)
LIB_BUILD_DIR = $(LIB_DIR)/build
LIB_NAME = c
LIB = $(LIB_BUILD_DIR)/lib$(LIB_NAME).a

# Flags
INCFLAGS = -I$(WORK_DIR)/include
COMMON_FLAGS = -fno-pic -mcmodel=medany
CFLAGS = $(COMMON_FLAGS) -static -O2 -MMD -Wall -ggdb $(INCFLAGS) \
         -fno-builtin -fno-stack-protector -ffreestanding -Wno-main

default: $(LIB)

$(LIB): $(OBJ)
	$(AR) cr $@ $^

# Compile: a single `.c` -> `.o` (gcc)
$(DST_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $(realpath $<)

# Compile: a single `.S` -> `.o` (gcc)
$(DST_DIR)/%.o: %.S
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -c -o $@ $(realpath $<)

clean:
	-rm -rf $(BUILD_DIR)

.PHONY: default clean
