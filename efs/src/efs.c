#include "efs.h"
#include "log.h"
#include "string.h"

void efs_create(EasyFileSystem *efs, BlockDevice *device, uint32_t total_blocks,
                uint32_t inode_bitmap_blocks) {
  bitmap_new(&efs->inode_bitmap, 1, inode_bitmap_blocks);
  uint64_t inode_num = bitmap_maximum(&efs->inode_bitmap);
  uint32_t inode_area_blocks =
      (inode_num * sizeof(DiskInode) + BLOCK_SZ - 1) / BLOCK_SZ;
  uint32_t inode_total_blocks = inode_bitmap_blocks + inode_area_blocks;
  uint32_t data_total_blocks = total_blocks - 1 - inode_total_blocks;
  uint32_t data_bitmap_blocks = (data_total_blocks + 4096) / 4097;
  uint32_t data_area_blocks = data_total_blocks - data_bitmap_blocks;
  bitmap_new(&efs->data_bitmap, (1 + inode_bitmap_blocks + inode_area_blocks),
             data_bitmap_blocks);

  efs->block_device = device;
  efs->inode_area_start_block = 1 + inode_bitmap_blocks;
  efs->data_area_start_block = 1 + inode_total_blocks + data_bitmap_blocks;

  BlockCache *bc;

  // clear all blocks
  for (uint32_t i = 0; i < total_blocks; i++) {
    bc = block_cache_get(i, device);
    bc->modified = true;
    memset(bc->cache, 0, BLOCK_SZ);
    block_cache_release(bc);
  }

  // initialize SuperBlock
  SuperBlock *sb;
  bc = block_cache_get(0, device);
  bc->modified = true;
  sb = (SuperBlock *)bc->cache;
  super_block_initialize(sb, total_blocks, inode_bitmap_blocks,
                         inode_area_blocks, data_bitmap_blocks,
                         data_area_blocks);
  block_cache_release(bc);

  // write back immediately
  // create an inode for root node "/"
  assert(efs_alloc_inode(efs) == 0);
  uint32_t root_inode_block_id = efs_get_disk_inode_id(efs, 0);
  uint64_t root_inode_offset = efs_get_disk_inode_offset(efs, 0);
  bc = block_cache_get(root_inode_block_id, device);
  bc->modified = true;
  DiskInode *disk_inode = (DiskInode *)(bc->cache + root_inode_offset);
  disk_inode_initialize(disk_inode, DISK_INODE_DIRECTORY);
  block_cache_release(bc);
  block_cache_sync_all();
}

void efs_open(EasyFileSystem *efs, BlockDevice *device) {
  // read SuperBlock
  BlockCache *bc;
  SuperBlock *sb;
  bc = block_cache_get(0, device);
  sb = (SuperBlock *)bc->cache;

  assert(super_block_is_valid(sb));
  uint32_t inode_total_blocks = sb->inode_bitmap_blocks + sb->inode_area_blocks;

  bitmap_new(&efs->inode_bitmap, 1, sb->inode_bitmap_blocks);
  bitmap_new(&efs->data_bitmap, 1 + inode_total_blocks, sb->data_bitmap_blocks);
  efs->inode_area_start_block = 1 + sb->inode_bitmap_blocks;
  efs->data_area_start_block = 1 + inode_total_blocks + sb->data_bitmap_blocks;
  efs->block_device = device;

  block_cache_release(bc);
}

void efs_root_inode(Inode *inode, EasyFileSystem *efs) {
  BlockDevice *block_device = efs->block_device;
  uint32_t block_id = efs_get_disk_inode_id(efs, 0);
  uint32_t block_offset = efs_get_disk_inode_offset(efs, 0);
  inode->block_id = block_id;
  inode->block_offset = block_offset;
  inode->fs = efs;
  inode->block_device = block_device;
}

uint32_t efs_get_disk_inode_id(EasyFileSystem *efs, uint32_t inode_id) {
  uint32_t inodes_per_block = BLOCK_SZ / sizeof(DiskInode);
  return efs->inode_area_start_block + inode_id / inodes_per_block;
}

uint64_t efs_get_disk_inode_offset(EasyFileSystem *efs, uint32_t inode_id) {
  uint32_t inodes_per_block = BLOCK_SZ / sizeof(DiskInode);
  return (inode_id % inodes_per_block) * sizeof(DiskInode);
}

uint32_t efs_get_data_block_id(EasyFileSystem *efs, uint32_t data_block_id) {
  return efs->data_area_start_block + data_block_id;
}

uint32_t efs_alloc_inode(EasyFileSystem *efs) {
  return bitmap_alloc(&efs->inode_bitmap, efs->block_device);
}

// Return a block ID in the data area
uint32_t efs_alloc_data(EasyFileSystem *efs) {
  return bitmap_alloc(&efs->data_bitmap, efs->block_device) +
         efs->data_area_start_block;
}

void efs_dealloc_data(EasyFileSystem *efs, uint32_t block_id) {
  BlockCache *bc;
  bc = block_cache_get(block_id, efs->block_device);
  bc->modified = true;
  memset(bc->cache, 0, BLOCK_SZ);
  block_cache_release(bc);
  bitmap_dealloc(&efs->data_bitmap, efs->block_device,
                 block_id - efs->data_area_start_block);
}
