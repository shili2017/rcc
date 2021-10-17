#ifndef _MM_H_
#define _MM_H_

#include <stdbool.h>
#include <stdint.h>

#include "config.h"

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

typedef struct LinkedList LinkedList;

struct LinkedList {
  LinkedList *next;
}

struct StackFrameAllocator {
  // PhysPageNum in rCore, but PhysAddr in rcc
  PhysAddr current;
  PhysAddr end;
  LinkedList *recycled;
}

typedef struct StackFrameAllocator StackFrameAllocator;

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
#define pte_flags(pte) ((PTEFlags)(pte & 0xff))
#define pte_is_valid(pte) ((pte & PTE_V) != pte_empty())
#define pte_readable(pte) ((pte & PTE_R) != pte_empty())
#define pte_writable(pte) ((pte & PTE_W) != pte_empty())
#define pte_executable(pte) ((pte & PTE_X) != pte_empty())

typedef PhysPageNum PageTable;

struct VPNRange {
  VirtPageNum l;
  VirtPageNum r;
}

typedef struct VPNRange VPNRange;

struct MapArea {
  VPNRange vpn_range;

}

struct MemorySet {
  PageTable page_table;
}

#endif // _MM_H_
