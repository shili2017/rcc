#include <stddef.h>

#include "log.h"
#include "mm.h"
#include "string.h"

void page_table_new(PageTable *pt) {
  PhysPageNum frame = frame_alloc();
  pt->root_ppn = frame;
}

void page_table_from_token(PageTable *pt, uint64_t satp) {
  pt->root_ppn = (PhysPageNum)(satp & ((1L << 44) - 1));
}

PageTableEntry *page_table_find_pte_create(PageTable *pt, VirtPageNum vpn) {
  uint64_t idxs[3];
  vpn_indexes(vpn, idxs);
  PhysPageNum ppn = pt->root_ppn;
  PageTableEntry *result = NULL;
  for (unsigned i = 0; i < 3; i++) {
    PageTableEntry *pte = ppn_get_pte_array(ppn) + idxs[i];
    if (i == 2) {
      result = pte;
      break;
    }
    if (!pte_is_valid(*pte)) {
      PhysPageNum frame = frame_alloc();
      if (!frame) {
        return NULL;
      }
      *pte = pte_new(frame, PTE_V);
    }
    ppn = pte_ppn(*pte);
  }
  return result;
}

PageTableEntry *page_table_find_pte(PageTable *pt, VirtPageNum vpn) {
  uint64_t idxs[3];
  vpn_indexes(vpn, idxs);
  PhysPageNum ppn = pt->root_ppn;
  PageTableEntry *result = NULL;
  for (unsigned i = 0; i < 3; i++) {
    PageTableEntry *pte = ppn_get_pte_array(ppn) + idxs[i];
    if (i == 2) {
      result = pte;
      break;
    }
    if (!pte_is_valid(*pte)) {
      return NULL;
    }
    ppn = pte_ppn(*pte);
  }
  return result;
}

void page_table_map(PageTable *pt, VirtPageNum vpn, PhysPageNum ppn,
                    PTEFlags flags) {
  PageTableEntry *pte = page_table_find_pte_create(pt, vpn);
  assert(!pte_is_valid(*pte), "vpn 0x%llx is mapped before mapping\n", vpn);
  *pte = pte_new(ppn, flags | PTE_V);
}

void page_table_unmap(PageTable *pt, VirtPageNum vpn) {
  PageTableEntry *pte = page_table_find_pte_create(pt, vpn);
  assert(pte_is_valid(*pte), "vpn 0x%llx is invalid before unmapping\n", vpn);
  *pte = pte_empty();
}

PageTableEntry *page_table_translate(PageTable *pt, VirtPageNum vpn) {
  return page_table_find_pte(pt, vpn);
}

uint64_t page_table_token(PageTable *pt) { return ((8L << 60) | pt->root_ppn); }

void copy_byte_buffer(uint64_t token, uint8_t *kernel, uint8_t *user,
                      uint64_t len, uint64_t direction) {
  if (direction != FROM_USER && direction != TO_USER) {
    panic("Unknown direction in copy_byte_buffer.\n");
  }

  PageTable page_table;
  page_table_from_token(&page_table, token);
  uint64_t start = (uint64_t)user;
  uint64_t end = start + len;
  uint64_t kernel_i = 0;

  VirtAddr start_va, end_va;
  VirtPageNum vpn;
  PhysPageNum ppn;
  uint8_t *bytes_array;

  while (start < end) {
    start_va = (VirtAddr)start;
    vpn = page_floor(start_va);
    ppn = pte_ppn(*page_table_translate(&page_table, vpn));
    vpn++;
    end_va = (VirtAddr)pn2addr(vpn);
    if ((VirtAddr)end < end_va) {
      end_va = (VirtAddr)end;
    }
    bytes_array = ppn_get_bytes_array(ppn);
    if (page_aligned(end_va)) {
      if (direction == FROM_USER)
        memcpy(kernel + kernel_i, bytes_array,
               PAGE_SIZE - page_offset(start_va));
      else
        memcpy(bytes_array, kernel + kernel_i,
               PAGE_SIZE - page_offset(start_va));
      kernel_i += PAGE_SIZE - page_offset(start_va);
    } else {
      if (direction == FROM_USER)
        memcpy(kernel + kernel_i, bytes_array + page_offset(start_va),
               page_offset(end_va) - page_offset(start_va));
      else
        memcpy(bytes_array + page_offset(start_va), kernel + kernel_i,
               page_offset(end_va) - page_offset(start_va));
      kernel_i += page_offset(end_va) - page_offset(start_va);
    }
    start = (uint64_t)end_va;
  }
}
