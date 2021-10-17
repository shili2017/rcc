#include <stdint.h>

#include "config.h"
#include "mm.h"
#include "string.h"

static uint8_t HEAP_SPACE[KERNEL_HEAP_SIZE] __attribute__((aligned(4096)));

void heap_allocator_init_heap() { memset(HEAP_SPACE, 0, KERNEL_HEAP_SIZE); }
