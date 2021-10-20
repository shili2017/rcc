#include "config.h"
#include "mm.h"

void vpn_indexes(VirtPageNum vpn, uint64_t *idx) {
  for (int i = 2; i >= 0; i--) {
    idx[i] = vpn & 0x1ff;
    vpn >>= 9;
  }
}

PageTableEntry *ppn_get_pte_array(PhysPageNum ppn) {
  PhysAddr pa = pn2addr(ppn);
  return (PageTableEntry *)pa; // len = 512 * 8
}

uint8_t *ppn_get_bytes_array(PhysPageNum ppn) {
  PhysAddr pa = pn2addr(ppn);
  return (uint8_t *)pa; // len = PAGE_SIZE (4096)
}
