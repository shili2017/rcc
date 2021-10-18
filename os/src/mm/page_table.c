#include <stddef.h>

#include "log.h"
#include "mm.h"

void page_table_new(PageTable *pt) {
  PhysPageNum frame = frame_alloc();
  pt->root_ppn = frame;
}

PageTableEntry *page_table_find_pte_create(PageTable *pt, VirtPageNum vpn) {
  uint64_t idxs[3];
  vpn_indexes(vpn, &idxs);
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
  vpn_indexes(vpn, &idxs);
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
