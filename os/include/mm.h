#ifndef _MM_H_
#define _MM_H_

#include <stdbool.h>
#include <stdint.h>

#include "config.h"
#include "external.h"

typedef uint64_t PhysAddr;
typedef uint64_t VirtAddr;
typedef uint64_t PhysPageNum;
typedef uint64_t VirtPageNum;

#define addr_floor(x) ((x) / PAGE_SIZE)
#define addr_ceil(x) (((x)-1 + PAGE_SIZE) / PAGE_SIZE)
#define addr_page_offset(x) ((x) & (PAGE_SIZE - 1))
#define addr_aligned(x) (addr_page_offset(x) == 0)
#define get_page_num_from_addr(x) addr_floor(x)
#define get_addr_from_page_num(x) ((x) << PAGE_SIZE_BITS)

typedef struct {
  PhysPageNum current;
  PhysPageNum end;
  struct vector recycled;
} StackFrameAllocator;

#define PTE_V (1L << 0)
#define PTE_R (1L << 1)
#define PTE_W (1L << 2)
#define PTE_X (1L << 3)
#define PTE_U (1L << 4)
#define PTE_G (1L << 5)
#define PTE_A (1L << 6)
#define PTE_D (1L << 7)

typedef uint8_t PTEFlags;
typedef uint64_t PageTableEntry;

#define pte_new(ppn, flags) ((ppn) << 10 | (PageTableEntry)flags)
#define pte_empty() 0
#define pte_ppn(pte) (((pte) >> 10) & ((1L << 44) - 1))
#define pte_flags(pte) ((PTEFlags)((pte)&0xff))
#define pte_is_valid(pte) (((pte)&PTE_V) != pte_empty())
#define pte_readable(pte) (((pte)&PTE_R) != pte_empty())
#define pte_writable(pte) (((pte)&PTE_W) != pte_empty())
#define pte_executable(pte) (((pte)&PTE_X) != pte_empty())

typedef struct {
  PhysPageNum root_ppn;
} PageTable;

void page_table_new(PageTable *pt);
PageTableEntry *page_table_find_pte_create(PageTable *pt, VirtPageNum vpn);
PageTableEntry *page_table_find_pte(PageTable *pt, VirtPageNum vpn);
void page_table_map(PageTable *pt, VirtPageNum vpn, PhysPageNum ppn,
                    PTEFlags flags);
void page_table_unmap(PageTable *pt, VirtPageNum vpn);
PageTableEntry *page_table_translate(PageTable *pt, VirtPageNum vpn);
uint64_t page_table_token(PageTable *pt);

typedef struct {
  VirtPageNum l;
  VirtPageNum r;
} VPNRange;

typedef uint8_t MapType;
#define MAP_IDENTICAL 0
#define MAP_FRAMED 1

typedef uint8_t MapPermission;
#define MAP_PERM_R PTE_R
#define MAP_PERM_W PTE_W
#define MAP_PERM_X PTE_X
#define MAP_PERM_U PTE_U

typedef struct {
  VPNRange vpn_range;
  MapType map_type;
  MapPermission map_perm;
} MapArea;

typedef struct {
  PageTable page_table;
  struct vector areas;
} MemorySet;

#endif // _MM_H_
