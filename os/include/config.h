#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <stdint.h>

#define USER_STACK_SIZE (4096 * 2)
#define KERNEL_STACK_SIZE (4096 * 2)
#define KERNEL_HEAP_SIZE 0x300000
#define MEMORY_END 0x80800000
#define PAGE_SIZE 0x1000
#define PAGE_SIZE_BITS 0xc
#define PAGE_SHIFT 12

#define TRAMPOLINE (UINT64_MAX - PAGE_SIZE + 1)
#define TRAP_CONTEXT (TRAMPOLINE - PAGE_SIZE)

#define APP_BASE_ADDRESS 0x80400000
#define APP_SIZE_LIMIT 0x20000

#define MAX_APP_SIZE (1024 * 1024)

// kernel stack in kernel space
#define kernel_stack_position_top(x)                                           \
  (TRAMPOLINE - (x) * (KERNEL_STACK_SIZE + PAGE_SIZE))
#define kernel_stack_position_bottom(x)                                        \
  (kernel_stack_position_top(x) - KERNEL_STACK_SIZE)

// clock freq for qemu
#define CLOCK_FREQ 10000000

// virtio mmio interface
#define VIRTIO0 0x10001000

#define MMIO_NUM 1
const static uint64_t MMIO[MMIO_NUM][2] = {{0x10001000, 0x1000}};

#endif // _CONFIG_H_
