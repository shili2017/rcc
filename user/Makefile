# Toolchain
ARCH = riscv64-unknown-elf
CC = $(ARCH)-gcc
AS = $(ARCH)-gcc
LD = $(ARCH)-ld
AR = $(ARCH)-ar
OBJDUMP = $(ARCH)-objdump
OBJCOPY = $(ARCH)-objcopy

# Directories
WORK_DIR = $(shell pwd)
BUILD_DIR = $(WORK_DIR)/build
DST_DIR = $(BUILD_DIR)/$(ARCH)
$(shell mkdir -p $(DST_DIR))

# Library (libc)
LIB_DIR = $(realpath ../libc)
LIB_BUILD_DIR = $(LIB_DIR)/build
LIB_NAME = c
LIB = $(LIB_BUILD_DIR)/lib$(LIB_NAME).a

# Build target 1: user_lib
USER_LIB_SRC = $(wildcard src/*.c src/*.S)
USER_LIB_OBJ = $(addprefix $(DST_DIR)/, $(addsuffix .o, $(basename $(USER_LIB_SRC))))
USER_LIB_NAME = user_lib
USER_LIB = $(BUILD_DIR)/lib$(USER_LIB_NAME).a

# Build target 2: user apps
APPS_DIR = src/bin
APPS_SRC = $(wildcard $(APPS_DIR)/*.c)
APPS = $(basename $(notdir $(APPS_SRC)))
APPS_OBJ = $(addprefix $(DST_DIR)/$(APPS_DIR)/, $(addsuffix .o, $(APPS)))
APPS_ELF = $(addprefix $(BUILD_DIR)/, $(addsuffix .elf, $(APPS)))
APPS_LINKER_SCRIPT = $(WORK_DIR)/src/linker.ld

# Build target 3: fs.img
FS_IMG = fs.img
EFS_FUSE = efs-fuse
EFS_FUSE_DIR = $(realpath ../efs-fuse)

# Flags
INCFLAGS = -I$(WORK_DIR)/include -I$(LIB_DIR)/include
COMMON_FLAGS = -fno-pic -mcmodel=medany
CFLAGS = $(COMMON_FLAGS) -static -O2 -MMD -Wall -ggdb $(INCFLAGS) \
         -fno-builtin -fno-stack-protector -ffreestanding -Wno-main
ASFLAGS = $(COMMON_FLAGS) -O0 -MMD $(INCFLAGS)
LDFLAGS = -melf64lriscv -T $(APPS_LINKER_SCRIPT) \
				  -L$(LIB_BUILD_DIR) -l$(LIB_NAME) -L$(BUILD_DIR) -l$(USER_LIB_NAME) -g

default: $(FS_IMG)

$(FS_IMG): build-lib $(USER_LIB) $(APPS_ELF) $(APPS_OBJ)
	cd $(EFS_FUSE_DIR) && $(MAKE)
	cp $(EFS_FUSE_DIR)/$(EFS_FUSE) .
	./$(EFS_FUSE) $@ $(APPS_ELF)

build-lib:
	cd $(LIB_DIR) && $(MAKE)

$(USER_LIB): $(USER_LIB_OBJ)
	$(AR) cr $@ $^

$(BUILD_DIR)/%.elf: $(DST_DIR)/$(APPS_DIR)/%.o
	$(LD) $(LDFLAGS) $^ $(LIB) -o $@
	$(OBJCOPY) $@ --strip-all -O binary $(basename $@).bin
	$(OBJDUMP) $@ -d > $(basename $@).txt

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
	-rm -rf efs-fuse

.PHONY: default build-lib clean
