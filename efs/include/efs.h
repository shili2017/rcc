#ifndef _EFS_H_
#define _EFS_H_

#include <stdbool.h>
#include <stdint.h>

// block_dev

#define BLOCK_SZ 512

typedef struct BlockCache BlockCache;

typedef struct {
  void (*read_block)(BlockCache *b);
  void (*write_block)(BlockCache *b);
} BlockDevice;

// block_cache.c

struct BlockCache {
  uint8_t cache[BLOCK_SZ];
  uint64_t block_id;
  BlockDevice *block_device;
  bool disk; // does disk "own" BlockCache?
  bool modified;
  BlockCache *prev;
  BlockCache *next;
  uint64_t ref;
};

void block_cache_manager_init();
BlockCache *block_cache_get(uint64_t block_id, BlockDevice *block_device);
void block_cache_release(BlockCache *b);
void block_cache_sync_all();

// bitmap.c

#define BLOCK_BITS (BLOCK_SZ * 8)

typedef uint64_t BitmapBlock; // size = 64

typedef struct {
  uint64_t start_block_id;
  uint64_t blocks;
} Bitmap;

void bitmap_new(Bitmap *b, uint64_t start_block_id, uint64_t blocks);
uint64_t bitmap_alloc(Bitmap *b, BlockDevice *device);
void bitmap_dealloc(Bitmap *b, BlockDevice *device, uint64_t bit);
uint64_t bitmap_maximum(Bitmap *b);

// layout.c

#define EFS_MAGIC 0x3b800001
#define INODE_DIRECT_COUNT 28
#define NAME_LENGTH_LIMIT 27
#define INODE_INDIRECT1_COUNT (BLOCK_SZ / 4)
#define INODE_INDIRECT2_COUNT (INODE_INDIRECT1_COUNT * INODE_INDIRECT1_COUNT)
#define DIRECT_BOUND INODE_DIRECT_COUNT
#define INDIRECT1_BOUND (DIRECT_BOUND + INODE_INDIRECT1_COUNT)
#define INDIRECT2_BOUND (INDIRECT1_BOUND + INODE_INDIRECT2_COUNT)

typedef struct {
  uint32_t magic;
  uint32_t total_blocks;
  uint32_t inode_bitmap_blocks;
  uint32_t inode_area_blocks;
  uint32_t data_bitmap_blocks;
  uint32_t data_area_blocks;
} SuperBlock;

void super_block_initialize(SuperBlock *s, uint32_t total_blocks,
                            uint32_t inode_bitmap_blocks,
                            uint32_t inode_area_blocks,
                            uint32_t data_bitmap_blocks,
                            uint32_t data_area_blocks);
bool super_block_is_valid(SuperBlock *s);

#define DISK_INODE_FILE 0
#define DISK_INODE_DIRECTORY 1

typedef uint32_t *IndirectBlock; // size = BLOCK_SZ / 4
typedef uint8_t *DataBlock;      // size = BLOCK_SZ

typedef struct {
  uint32_t size;
  uint32_t direct[INODE_DIRECT_COUNT];
  uint32_t indirect1;
  uint32_t indirect2;
  uint32_t type_;
} DiskInode;

void disk_inode_initialize(DiskInode *d, uint32_t type_);
bool disk_inode_is_dir(DiskInode *d);
bool disk_inode_is_file(DiskInode *d);
uint32_t disk_inode_data_blocks(DiskInode *d);
uint32_t disk_inode_total_blocks(uint32_t size);
uint32_t disk_inode_blocks_num_needed(DiskInode *d, uint32_t new_size);
uint32_t disk_inode_get_block_id(DiskInode *d, uint32_t inner_id,
                                 BlockDevice *device);
void disk_inode_increase_size(DiskInode *d, uint32_t new_size,
                              uint32_t *new_blocks, BlockDevice *device);
void disk_inode_clear_size(DiskInode *d, uint32_t *v, BlockDevice *device);
uint64_t disk_inode_read_at(DiskInode *d, uint64_t offset, uint8_t *buf,
                            uint64_t len, BlockDevice *device);
uint64_t disk_inode_write_at(DiskInode *d, uint64_t offset, uint8_t *buf,
                             uint64_t len, BlockDevice *device);

#define DIRENT_SZ 32

typedef struct {
  char name[NAME_LENGTH_LIMIT + 1];
  uint32_t inode_number;
} DirEntry;

void dir_entry_empty(DirEntry *e);
void dir_entry_new(DirEntry *e, char *name, uint32_t inode_number);

// efs.c

typedef struct {
  BlockDevice *block_device;
  Bitmap inode_bitmap;
  Bitmap data_bitmap;
  uint32_t inode_area_start_block;
  uint32_t data_area_start_block;
} EasyFileSystem;

typedef struct {
  uint64_t block_id;
  uint64_t block_offset;
  EasyFileSystem *fs;
  BlockDevice *block_device;
} Inode;

void efs_create(EasyFileSystem *efs, BlockDevice *device, uint32_t total_blocks,
                uint32_t inode_bitmap_blocks);
void efs_open(EasyFileSystem *efs, BlockDevice *device);
void efs_root_inode(Inode *inode, EasyFileSystem *efs);
uint32_t efs_get_disk_inode_id(EasyFileSystem *efs, uint32_t inode_id);
uint64_t efs_get_disk_inode_offset(EasyFileSystem *efs, uint32_t inode_id);
uint32_t efs_get_data_block_id(EasyFileSystem *efs, uint32_t data_block_id);
uint32_t efs_alloc_inode(EasyFileSystem *efs);
uint32_t efs_alloc_data(EasyFileSystem *efs);
void efs_dealloc_data(EasyFileSystem *efs, uint32_t block_id);

// vfs.c

int64_t inode_find(Inode *inode, char *name, Inode *inode_found);
void inode_increase_size(Inode *inode, uint32_t new_size, DiskInode *disk_inode,
                         EasyFileSystem *fs);
int64_t inode_create(Inode *inode, char *name, Inode *inode_created);
void inode_ls(Inode *inode, char **v);
uint64_t inode_ls_len(Inode *inode);
uint64_t inode_read_at(Inode *inode, uint64_t offset, uint8_t *buf,
                       uint64_t len);
uint64_t inode_write_at(Inode *inode, uint64_t offset, uint8_t *buf,
                        uint64_t len);
void inode_clear(Inode *inode);

#endif // _EFS_H_
