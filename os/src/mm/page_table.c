#include "mm.h"

PageTable page_table_new() {
  PhysPageNum frame = frame_alloc();
  return (PageTable)frame;
}

PageTable page_table_from_token(uint64_t satp) {
  return (satp & ((1L << 44) - 1));
}

PageTableEntry page_table_find_pte_create(PageTable pt, VirtPageNum vpn) {
  uint64_t idxs[3];
  vpn_indexes(vpn, &idxs);
  PhysPageNum ppn = (PhysPageNum)pt;
  PageTableEntry result = NULL;
  for (unsigned i = 0; i < 3; i++) {
    PageTableEntry pte = ppn_get_pte_array(ppn)[idxs[i]];
    if (i == 2) {
      result = pte;
      break;
    }
    if (!pte_is_valid(pte)) {
      PhysPageNum frame = frame_alloc();
      if (!frame) {
        return NULL;
      }
      *pte = pte_new(frame, PTE_V);
    }
    ppn = pte_ppn(pte);
  }
  return result;
}

PageTableEntry page_table_find_pte(PageTable pt, VirtPageNum vpn) {
  uint64_t idxs[3];
  vpn_indexes(vpn, &idxs);
  PhysPageNum ppn = (PhysPageNum)pt;
  PageTableEntry result = NULL;
  for (unsigned i = 0; i < 3; i++) {
    PageTableEntry pte = ppn_get_pte_array(ppn)[idxs[i]];
    if (i == 2) {
      result = pte;
      break;
    }
    if (!pte_is_valid(pte)) {
      return NULL;
    }
    ppn = pte_ppn(pte);
  }
  return result;
}

void page_table_map(PageTable pt, VirtPageNum vpn, PhysPageNum ppn,
                    PTEFlags flags) {
  PageTableEntry pte = page_table_find_pte_create(pt, vpn);
  assert(!pte_is_valid(pte), "vpn 0x%llx is mapped before mapping\n", vpn);
  *pte = pte_new(ppn, flags | PTE_V);
}

void page_table_unmap(PageTable pt, VirtPageNum vpn) {
  PageTableEntry pte = page_table_find_pte_create(pt, vpn);
  assert(pte_is_valid(pte), "vpn 0x%llx is invalid before unmapping\n", vpn);
  *pte = pte_empty();
}

uint64_t page_table_token(PageTable pt) { return ((8L << 60) | pt) }
