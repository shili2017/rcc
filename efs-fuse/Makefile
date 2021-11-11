CC = gcc

WORK_DIR = $(shell pwd)
BUILD_DIR = $(WORK_DIR)/build/efs-fuse
TARGET = efs-fuse

SRC = $(wildcard ../efs/src/*.c) main.c

OBJ = $(addprefix $(BUILD_DIR)/, $(addsuffix .o, $(basename $(SRC))))

CFLAGS = -static -O0 -Wall -g
INCFLAGS = -I$(WORK_DIR)/../efs/include -I$(WORK_DIR)/../os/include

default: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^

$(BUILD_DIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCFLAGS) -c -o $@ $(realpath $<)

clean:
	-rm -rf build
	-rm $(TARGET)

.PHONY: default clean
