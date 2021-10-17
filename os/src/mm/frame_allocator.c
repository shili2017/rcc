#include "config.h"
#include "mm.h"
#include "string.h"

static StackFrameAllocator FRAME_ALLOCATOR;

void frame_allocator_init() {
  extern uint8_t ekernel;
  FRAME_ALLOCATOR.current = addr_ceil(&ekernel);
  FRAME_ALLOCATOR.end = addr_floor(MEMORY_END);
  FRAME_ALLOCATOR.recycled = NULL;
}

PhysPageNum frame_alloc() {
  if (FRAME_ALLOCATOR.recycled) {
    PhysPageNum ppn = get_page_num_from_addr(FRAME_ALLOCATOR.recycled);
    FRAME_ALLOCATOR.recycled = FRAME_ALLOCATOR.recycled->next;
    return ppn;
  } else {
    if (FRAME_ALLOCATOR.current == FRAME_ALLOCATOR.end) {
      return NULL;
    } else {
      PhysPageNum ppn = get_page_num_from_addr(FRAME_ALLOCATOR.current);
      FRAME_ALLOCATOR.current += PAGE_SIZE;
      return ppn;
    }
  }
}

void frame_dealloc(PhysPageNum ppn) {
  PhysAddr pa = get_addr_from_page_num(ppn);
  bool in_recycled = false;
  LinkedList *v = FRAME_ALLOCATOR.recycled;
  while (v) {
    if (v == pa) {
      in_recycled = true;
    } else {
      v = v->next;
    }
  }
  if (pa >= FRAME_ALLOCATOR.current || in_recycled) {
    panic("Frame ppn=%llx has not been allocated!\n", ppn);
  }
  v = FRAME_ALLOCATOR.recycled;
  FRAME_ALLOCATOR.recycled = (LinkedList *)pa;
  pa->next = v;
}
