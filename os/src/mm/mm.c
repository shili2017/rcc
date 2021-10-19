#include "mm.h"

void mm_init() {
  // mm init
  heap_allocator_init();
  frame_allocator_init();
  memory_set_kernel_init();
}

void mm_remap_test() {
  // mm remap_test
  memory_set_remap_test();
}
