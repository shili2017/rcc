#include "config.h"
#include "external.h"
#include "log.h"
#include "mm.h"
#include "string.h"

typedef struct {
  PhysPageNum current;
  PhysPageNum end;
  struct vector recycled;
} StackFrameAllocator;

static StackFrameAllocator FRAME_ALLOCATOR;

void frame_allocator_init() {
  extern uint8_t ekernel;
  FRAME_ALLOCATOR.current = page_ceil((PhysAddr)&ekernel);
  FRAME_ALLOCATOR.end = page_floor(MEMORY_END);
  vector_new(&FRAME_ALLOCATOR.recycled, sizeof(PhysPageNum));
}

void frame_allocator_free() { vector_free(&FRAME_ALLOCATOR.recycled); }

PhysPageNum frame_alloc() {
  PhysPageNum ppn = 0;
  if (!vector_empty(&FRAME_ALLOCATOR.recycled)) {
    ppn = *(PhysPageNum *)vector_back(&FRAME_ALLOCATOR.recycled);
    vector_pop(&FRAME_ALLOCATOR.recycled);
  } else {
    if (FRAME_ALLOCATOR.current == FRAME_ALLOCATOR.end) {
      panic("No empty physical page.\n");
    } else {
      ppn = FRAME_ALLOCATOR.current;
      FRAME_ALLOCATOR.current++;
    }
  }
  memset((uint8_t *)pn2addr(ppn), 0, PAGE_SIZE);
  return ppn;
}

void frame_dealloc(PhysPageNum ppn) {
  bool in_recycled = false;
  PhysPageNum *x = (PhysPageNum *)(FRAME_ALLOCATOR.recycled.buffer);
  for (uint64_t i = 0; i < FRAME_ALLOCATOR.recycled.size; i++) {
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

uint64_t frame_remaining_pages() {
  uint64_t x = (uint64_t)(FRAME_ALLOCATOR.end - FRAME_ALLOCATOR.current);
  uint64_t y = (uint64_t)FRAME_ALLOCATOR.recycled.size;
  return x + y;
}

void frame_allocator_print() {
  PhysPageNum *x = (PhysPageNum *)(FRAME_ALLOCATOR.recycled.buffer);
  printf("FRAME_ALLOCATOR current = %llx, recycled = [ ",
         FRAME_ALLOCATOR.current);
  for (uint64_t i = 0; i < FRAME_ALLOCATOR.recycled.size; i++) {
    printf("%llx ", x[i]);
  }
  printf("]\n");
}
