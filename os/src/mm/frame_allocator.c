#include "config.h"
#include "external.h"
#include "mm.h"
#include "string.h"

static StackFrameAllocator FRAME_ALLOCATOR;

void frame_allocator_init() {
  extern uint8_t ekernel;
  FRAME_ALLOCATOR.current = addr_ceil((PhysAddr)&ekernel);
  FRAME_ALLOCATOR.end = addr_floor(MEMORY_END);
  vector_new(&FRAME_ALLOCATOR.recycled, sizeof(PhysPageNum));
}

PhysPageNum frame_alloc() {
  PhysPageNum ppn;
  if (!vector_empty(&FRAME_ALLOCATOR.recycled)) {
    ppn = get_page_num_from_addr(
        *(PhysPageNum *)vector_back(&FRAME_ALLOCATOR.recycled));
    vector_pop(&FRAME_ALLOCATOR.recycled);
  } else {
    if (FRAME_ALLOCATOR.current == FRAME_ALLOCATOR.end) {
      panic("No empty physical page.\n");
    } else {
      ppn = get_page_num_from_addr(FRAME_ALLOCATOR.current);
      FRAME_ALLOCATOR.current++;
    }
  }
  memset(get_addr_from_page_num(ppn), 0, PAGE_SIZE);
  return ppn;
}

void frame_dealloc(PhysPageNum ppn) {
  bool in_recycled = false;
  PhysPageNum *x = (PhysPageNum *)(FRAME_ALLOCATOR.recycled.buffer);
  for (unsigned i = 0; i < FRAME_ALLOCATOR.recycled.size; i++) {
    if (x[i] == ppn) {
      in_recycled = true;
      break;
    }
  }
  if (ppn >= FRAME_ALLOCATOR.current || in_recycled) {
    panic("Frame ppn=%llx has not been allocated!\n", ppn);
  }
  vector_push(&FRAME_ALLOCATOR.recycled, &ppn);
}
