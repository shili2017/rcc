# Toolchain
ARCH = riscv64-unknown-elf
CC = $(ARCH)-gcc
AS = $(ARCH)-gcc
LD = $(ARCH)-ld
AR = $(ARCH)-ar

# Directories
WORK_DIR = $(shell pwd)
BUILD_DIR = $(WORK_DIR)/build
DST_DIR =
ifeq ($(KERNEL), 1)
DST_DIR = $(BUILD_DIR)/$(ARCH)/os
else
DST_DIR = $(BUILD_DIR)/$(ARCH)/user
endif
$(shell mkdir -p $(DST_DIR))

# Source
SRC = $(wildcard src/*.c src/*.S)
OBJ = $(addprefix $(DST_DIR)/, $(addsuffix .o, $(basename $(SRC))))

# Flags
INCFLAGS = -I$(WORK_DIR)/include
ifeq ($(KERNEL), 1)
INCFLAGS += -I$(WORK_DIR)/../os/include
else
INCFLAGS += -I$(WORK_DIR)/../user/include
endif
COMMON_FLAGS = -fno-pic -mcmodel=medany
CFLAGS = $(COMMON_FLAGS) -static -O2 -MMD -Wall -ggdb $(INCFLAGS) \
         -fno-builtin -fno-stack-protector -ffreestanding -Wno-main

# Build target
LIB_NAME = c
ifeq ($(KERNEL), 1)
LIB_NAME = kernelc
CFLAGS += -D__KERNEL__
endif
LIB = $(BUILD_DIR)/lib$(LIB_NAME).a

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
