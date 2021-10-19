#include "elfparse.h"
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

void memory_set_from_elf(MemorySet *memory_set, uint8_t *elf_data,
                         size_t elf_size, uint64_t *user_sp,
                         uint64_t *entry_point) {
  memory_set_new_bare(memory_set);

  // map trampoline
  memory_set_map_trampoline(memory_set);

  // map progam headers of elf, with U flag
  t_elf elf;
  int elf_load_ret = elf_load(elf_data, elf_size, &elf);
  if (elf_load_ret != 0) {
    panic("Elf load error.\n");
  }

  size_t ph_count = elf_header_get_phnum(&elf);
  VirtAddr start_va, end_va;
  MapPermission map_perm;
  uint64_t ph_flags;
  MapArea map_area;
  VirtPageNum max_end_vpn = 0;
  for (size_t i = 0; i < ph_count; i++) {
    t_elf_program *ph = &elf.programs[i];
    if (elf_program_get_type(&elf, ph) == PT_LOAD) {
      start_va = (VirtAddr)elf_program_get_vaddr(&elf, ph);
      end_va = (VirtAddr)(start_va + elf_program_get_memsz(&elf, ph));
      map_perm = MAP_PERM_U;
      ph_flags = elf_program_get_flags(&elf, ph);
      if (ph_flags | PF_R) {
        map_perm |= MAP_PERM_R;
      }
      if (ph_flags | PF_W) {
        map_perm |= MAP_PERM_W;
      }
      if (ph_flags | PF_X) {
        map_perm |= MAP_PERM_X;
      }
      map_area.vpn_range.l = addr_floor(start_va);
      map_area.vpn_range.r = addr_ceil(end_va);
      map_area.map_type = MAP_FRAMED;
      map_area.map_perm = map_perm;
      max_end_vpn = map_area.vpn_range.r;
      memory_set_push(memory_set, &map_area,
                      elf_data + elf_program_get_offset(&elf, ph),
                      elf_program_get_filesz(&elf, ph));
    }
  }

  // map user stack with U flags
  VirtAddr max_end_va = get_addr_from_page_num(max_end_vpn);
  VirtAddr user_stack_bottom = max_end_va;
  // guard page
  user_stack_bottom += PAGE_SIZE;
  VirtAddr user_stack_top = user_stack_bottom + USER_STACK_SIZE;
  map_area.vpn_range.l = addr_floor(user_stack_bottom);
  map_area.vpn_range.r = addr_ceil(user_stack_top);
  map_area.map_type = MAP_FRAMED;
  map_area.map_perm = MAP_PERM_R | MAP_PERM_W | MAP_PERM_U;
  memory_set_push(memory_set, &map_area, NULL, 0);

  // map TrapContext
  map_area.vpn_range.l = addr_floor(TRAP_CONTEXT);
  map_area.vpn_range.r = addr_ceil(TRAMPOLINE);
  map_area.map_type = MAP_FRAMED;
  map_area.map_perm = MAP_PERM_R | MAP_PERM_W;
  memory_set_push(memory_set, &map_area, NULL, 0);

  // return
  *user_sp = (uint64_t)user_stack_top;
  *entry_point = elf_header_get_entry(&elf);
}

void memory_set_activate(MemorySet *memory_set) {
  uint64_t satp = page_table_token(&memory_set->page_table);
  w_satp(satp);
  sfence_vma();
}

PageTableEntry *memory_set_translate(MemorySet *memory_set, VirtPageNum vpn) {
  return page_table_translate(&memory_set->page_table, vpn);
}
