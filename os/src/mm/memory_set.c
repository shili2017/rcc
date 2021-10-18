#include "log.h"
#include "mm.h"
#include "riscv.h"
#include "string.h"

void map_area_map_one(MapArea *map_area, PageTable *pt, VirtPageNum vpn) {
  PhysPageNum ppn;
  if (map_area->map_type == MAP_IDENTICAL) {
    ppn = (PhysPageNum)vpn;
  } else {
    ppn = frame_alloc();
  }
  PTEFlags pte_flags = (PTEFlags)(map_area->map_perm);
  page_table_map(pt, vpn, ppn, pte_flags);
}

void map_area_unmap_one(MapArea *map_area, PageTable *pt, VirtPageNum vpn) {
  page_table_unmap(pt, vpn);
}

void map_area_map(MapArea *map_area, PageTable *pt) {
  for (VirtPageNum vpn = map_area->vpn_range.l; vpn < map_area->vpn_range.r;
       vpn++) {
    map_area_map_one(map_area, pt, vpn);
  }
}

void map_area_unmap(MapArea *map_area, PageTable *pt) {
  for (VirtPageNum vpn = map_area->vpn_range.l; vpn < map_area->vpn_range.r;
       vpn++) {
    map_area_unmap_one(map_area, pt, vpn);
  }
}

void map_area_copy_data(MapArea *map_area, PageTable *pt, uint8_t *data,
                        uint64_t len) {
  uint64_t start = 0;
  VirtPageNum current_vpn = map_area->vpn_range.l;
  for (;;) {
    uint8_t *src = &data[start];
    uint8_t *dst =
        ppn_get_bytes_array(pte_ppn(*page_table_translate(pt, current_vpn)));
    uint64_t cpy_len = (len - start >= PAGE_SIZE) ? PAGE_SIZE : (len - start);
    memcpy(dst, src, cpy_len);
    start += PAGE_SIZE;
    if (start >= len) {
      break;
    }
    current_vpn += 1;
  }
}

void memory_set_new_bare(MemorySet *memory_set) {
  page_table_new(&memory_set->page_table);
  vector_new(&memory_set->areas, sizeof(MapArea));
}

uint64_t memory_set_token(MemorySet *memory_set) {
  return page_table_token(&memory_set->page_table);
}

void memory_set_push(MemorySet *memory_set, MapArea *map_area, uint8_t *data,
                     uint64_t len) {
  map_area_map(map_area, &memory_set->page_table);
  if (data && len >= 0) {
    map_area_copy_data(map_area, &memory_set->page_table, data, len);
  }
  vector_push(&memory_set->areas, map_area);
}

// Assume that no conflicts.
void memory_set_insert_framed_area(MemorySet *memory_set, VirtAddr start_va,
                                   VirtAddr end_va, MapPermission permission) {
  MapArea map_area;
  map_area.vpn_range.l = addr_floor(start_va);
  map_area.vpn_range.r = addr_ceil(end_va);
  map_area.map_type = MAP_FRAMED;
  map_area.map_perm = permission;
  memory_set_push(memory_set, &map_area, NULL, 0);
}

extern uint8_t stext;
extern uint8_t etext;
extern uint8_t srodata;
extern uint8_t erodata;
extern uint8_t sdata;
extern uint8_t edata;
extern uint8_t sbss_with_stack;
extern uint8_t ebss;
extern uint8_t ekernel;
extern uint8_t strampoline;

// Mention that trampoline is not collected by areas.
void memory_set_map_trampoline(MemorySet *memory_set) {
  page_table_map(&memory_set->page_table, get_page_num_from_addr(TRAMPOLINE),
                 get_page_num_from_addr((PhysAddr)&strampoline), PTE_R | PTE_X);
}

static MemorySet KERNEL_SPACE;

void memory_set_new_kernel() {
  MemorySet *memory_set = &KERNEL_SPACE;
  memory_set_new_bare(memory_set);

  // map trampoline
  memory_set_map_trampoline(memory_set);

  // map kernel sections
  info(".text      [0x%llx, 0x%llx)\n", &stext, &etext);
  info(".rodata    [0x%llx, 0x%llx)\n", &srodata, &erodata);
  info(".data      [0x%llx, 0x%llx)\n", &sdata, &edata);
  info("boot_stack [0x%llx, 0x%llx)\n", &sbss_with_stack, &ebss);

  MapArea map_area;

  info("mapping .text section\n");
  map_area.vpn_range.l = addr_floor((PhysAddr)&stext);
  map_area.vpn_range.r = addr_ceil((PhysAddr)&etext);
  map_area.map_type = MAP_IDENTICAL;
  map_area.map_perm = MAP_PERM_R | MAP_PERM_X;
  memory_set_push(memory_set, &map_area, NULL, 0);

  info("mapping .rodata section\n");
  map_area.vpn_range.l = addr_floor((PhysAddr)&srodata);
  map_area.vpn_range.r = addr_ceil((PhysAddr)&erodata);
  map_area.map_type = MAP_IDENTICAL;
  map_area.map_perm = MAP_PERM_R;
  memory_set_push(memory_set, &map_area, NULL, 0);

  info("mapping .data section\n");
  map_area.vpn_range.l = addr_floor((PhysAddr)&sdata);
  map_area.vpn_range.r = addr_ceil((PhysAddr)&edata);
  map_area.map_type = MAP_IDENTICAL;
  map_area.map_perm = MAP_PERM_R | MAP_PERM_W;
  memory_set_push(memory_set, &map_area, NULL, 0);

  info("mapping .bss section\n");
  map_area.vpn_range.l = addr_floor((PhysAddr)&sbss_with_stack);
  map_area.vpn_range.r = addr_ceil((PhysAddr)&ebss);
  map_area.map_type = MAP_IDENTICAL;
  map_area.map_perm = MAP_PERM_R | MAP_PERM_W;
  memory_set_push(memory_set, &map_area, NULL, 0);

  info("mapping physical memory\n");
  map_area.vpn_range.l = addr_floor((PhysAddr)&ekernel);
  map_area.vpn_range.r = addr_ceil((PhysAddr)MEMORY_END);
  map_area.map_type = MAP_IDENTICAL;
  map_area.map_perm = MAP_PERM_R | MAP_PERM_W;
  memory_set_push(memory_set, &map_area, NULL, 0);
}

void memory_set_activate(MemorySet *memory_set) {
  uint64_t satp = page_table_token(&memory_set->page_table);
  w_satp(satp);
  sfence_vma();
}

PageTableEntry *memory_set_translate(MemorySet *memory_set, VirtPageNum vpn) {
  return page_table_translate(&memory_set->page_table, vpn);
}
