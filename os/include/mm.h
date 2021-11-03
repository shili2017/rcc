#ifndef _MM_H_
#define _MM_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "config.h"
#include "external.h"

#define MMAP_MAX_SIZE (1 << 30) // 1 GB

#define page_floor(x) ((x) / PAGE_SIZE)
#define page_ceil(x) (((x)-1 + PAGE_SIZE) / PAGE_SIZE)
#define page_offset(x) ((x) & (PAGE_SIZE - 1))
#define page_aligned(x) (page_offset(x) == 0)
#define addr2pn(x) page_floor(x)
#define pn2addr(x) ((x) << PAGE_SIZE_BITS)

#define pte_new(ppn, flags) ((ppn) << 10 | (PageTableEntry)flags)
#define pte_empty() 0
#define pte_ppn(pte) (((pte) >> 10) & ((1L << 44) - 1))
#define pte_flags(pte) ((PTEFlags)((pte)&0xff))
#define pte_is_valid(pte) (((pte)&PTE_V) != pte_empty())
#define pte_readable(pte) (((pte)&PTE_R) != pte_empty())
#define pte_writable(pte) (((pte)&PTE_W) != pte_empty())
#define pte_executable(pte) (((pte)&PTE_X) != pte_empty())

#define PTE_V (1L << 0)
#define PTE_R (1L << 1)
#define PTE_W (1L << 2)
#define PTE_X (1L << 3)
#define PTE_U (1L << 4)
#define PTE_G (1L << 5)
#define PTE_A (1L << 6)
#define PTE_D (1L << 7)

#define MAP_IDENTICAL 0
#define MAP_FRAMED 1

#define MAP_PERM_R PTE_R
#define MAP_PERM_W PTE_W
#define MAP_PERM_X PTE_X
#define MAP_PERM_U PTE_U

#define FROM_USER 0
#define TO_USER 1

typedef uint64_t PhysAddr;
typedef uint64_t VirtAddr;
typedef uint64_t PhysPageNum;
typedef uint64_t VirtPageNum;

typedef uint8_t PTEFlags;
typedef uint64_t PageTableEntry;

typedef struct {
  PhysPageNum root_ppn;
  struct vector frames;
} PageTable;

typedef struct {
  VirtPageNum l;
  VirtPageNum r;
} VPNRange;

typedef uint8_t MapType;
typedef uint8_t MapPermission;

typedef struct {
  VPNRange vpn_range;
  MapType map_type;
  MapPermission map_perm;
} MapArea;

typedef struct {
  PageTable page_table;
  struct vector areas;
} MemorySet;

/*  Data structure in rcc memory management
 *
 *  MemorySet
 *    ├── PageTable
 *    │     ├── Root PPN
 *    │     └── Vector of PPN frames
 *    └── Vector of MapArea
 *                    ├── VPN Range
 *                    │     ├── VPN l
 *                    │     └── VPN r
 *                    ├── MapType (Identical / Framed)
 *                    └── MapPermission (R / W / X / U)
 */

// mm.c
void mm_init();
void mm_free();
void mm_remap_test();

// address.c
void vpn_indexes(VirtPageNum vpn, uint64_t *idx);
PageTableEntry *ppn_get_pte_array(PhysPageNum ppn);
uint8_t *ppn_get_bytes_array(PhysPageNum ppn);

// heap_allocator.c
void heap_allocator_init();

// frame_allocator.c
void frame_allocator_init();
void frame_allocator_free();
PhysPageNum frame_alloc();
void frame_dealloc(PhysPageNum ppn);
uint64_t frame_remaining_pages();
void frame_allocator_print();

// page_table.c
void page_table_new(PageTable *pt);
void page_table_free(PageTable *pt);
void page_table_from_token(PageTable *pt, uint64_t satp);
PageTableEntry *page_table_find_pte_create(PageTable *pt, VirtPageNum vpn);
PageTableEntry *page_table_find_pte(PageTable *pt, VirtPageNum vpn);
void page_table_map(PageTable *pt, VirtPageNum vpn, PhysPageNum ppn,
                    PTEFlags flags);
void page_table_unmap(PageTable *pt, VirtPageNum vpn);
PageTableEntry *page_table_translate(PageTable *pt, VirtPageNum vpn);
uint64_t page_table_token(PageTable *pt);
int64_t copy_byte_buffer(uint64_t token, uint8_t *kernel, uint8_t *user,
                         uint64_t len, uint64_t direction);

// memory_set.c
uint64_t memory_set_token(MemorySet *memory_set);
void memory_set_free(MemorySet *memory_set);
void memory_set_from_elf(MemorySet *memory_set, uint8_t *elf_data,
                         size_t elf_size, uint64_t *user_sp,
                         uint64_t *entry_point);
void memory_set_from_existed_user(MemorySet *memory_set, MemorySet *user_space);
void memory_set_kernel_init();
PageTableEntry *memory_set_translate(MemorySet *memory_set, VirtPageNum vpn);
void memory_set_recycle_data_pages(MemorySet *memory_set);
void kernel_space_insert_framed_area(VirtAddr start_va, VirtAddr end_va,
                                     MapPermission permission);
void kernel_space_remove_area_with_start_vpn(VirtPageNum start_vpn);
uint64_t kernel_space_token();
int64_t memory_set_mmap(MemorySet *memory_set, uint64_t start, uint64_t len,
                        uint64_t prot);
int64_t memory_set_munmap(MemorySet *memory_set, uint64_t start, uint64_t len);
void memory_set_remap_test();

#endif // _MM_H_
