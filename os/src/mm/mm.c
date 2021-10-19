#include "mm.h"

void mm_init() {
  heap_allocator_init_heap();

  frame_allocator_init_frame_allocator();

  memory_set_kernel_init();
}
