#include "efs.h"
#include "external.h"
#include "log.h"
#include "string.h"

// return UINT32_MAX (-1) when cannot find inode id
static uint32_t inode_find_inode_id(Inode *inode, char *name,
                                    DiskInode *disk_inode) {
  // assert it is a directory
  assert(disk_inode_is_dir(disk_inode));
  uint64_t file_count = disk_inode->size / DIRENT_SZ;

  DirEntry dirent;
  dir_entry_empty(&dirent);

  for (uint64_t i = 0; i < file_count; i++) {
    assert(disk_inode_read_at(disk_inode, DIRENT_SZ * i, (uint8_t *)&dirent,
                              DIRENT_SZ, inode->block_device) == DIRENT_SZ);
    if (strcmp(dirent.name, name) == 0) {
      return dirent.inode_number;
    }
  }
  return UINT32_MAX;
}

// return 0 if found, -1 if not found
int64_t inode_find(Inode *inode, char *name, Inode *inode_found) {
  EasyFileSystem *fs = inode->fs;
  BlockCache *bc = block_cache_get(inode->block_id, inode->block_device);
  DiskInode *di = (DiskInode *)(bc->cache + inode->block_offset);
  uint32_t inode_id = inode_find_inode_id(inode, name, di);
  int64_t ret;
  if (inode_id == UINT32_MAX) {
    ret = -1;
  } else {
    ret = 0;
    inode_found->block_id = efs_get_disk_inode_id(fs, inode_id);
    inode_found->block_offset = efs_get_disk_inode_offset(fs, inode_id);
    inode_found->fs = fs;
    inode_found->block_device = inode->block_device;
  }
  block_cache_release(bc);
  return ret;
}

#ifdef __KERNEL__
extern void *bd_malloc(uint64_t);
extern void bd_free(void *);
#define MALLOC(x) bd_malloc(x)
#define FREE(x) bd_free(x)
#else
#include <stdlib.h>
#define MALLOC(x) malloc(x)
#define FREE(x) free(x)
#endif

void inode_increase_size(Inode *inode, uint32_t new_size, DiskInode *disk_inode,
                         EasyFileSystem *fs) {
  if (new_size < disk_inode->size) {
    return;
  }
  uint32_t blocks_needed = disk_inode_blocks_num_needed(disk_inode, new_size);
  uint32_t *v = MALLOC(sizeof(uint32_t) * blocks_needed);
  for (uint64_t i = 0; i < blocks_needed; i++) {
    v[i] = efs_alloc_data(fs);
  }
  disk_inode_increase_size(disk_inode, new_size, v, inode->block_device);
  FREE(v);
}

// return 0 if created, -1 if not created
int64_t inode_create(Inode *inode, char *name, Inode *inode_created) {
  EasyFileSystem *fs = inode->fs;
  BlockCache *bc;

  bc = block_cache_get(inode->block_id, inode->block_device);
  DiskInode *root_inode = (DiskInode *)(bc->cache + inode->block_offset);
  // assert it is a directory
  assert(disk_inode_is_dir(root_inode));
  // has the file been created?
  uint32_t inode_id = inode_find_inode_id(inode, name, root_inode);
  block_cache_release(bc);
  if (inode_id != UINT32_MAX) {
    return -1;
  }

  // create a new file
  // alloc a inode with an indirect block
  uint32_t new_inode_id = efs_alloc_inode(fs);
  // initialize inode
  uint32_t new_inode_block_id = efs_get_disk_inode_id(fs, new_inode_id);
  uint32_t new_inode_block_offset = efs_get_disk_inode_offset(fs, new_inode_id);
  bc = block_cache_get(new_inode_block_id, inode->block_device);
  bc->modified = true;
  DiskInode *new_inode = (DiskInode *)(bc->cache + new_inode_block_offset);
  disk_inode_initialize(new_inode, DISK_INODE_FILE);
  block_cache_release(bc);

  bc = block_cache_get(inode->block_id, inode->block_device);
  root_inode = (DiskInode *)(bc->cache + inode->block_offset);
  // append file in the dirent
  uint64_t file_count = root_inode->size / DIRENT_SZ;
  uint64_t new_size = (file_count + 1) * DIRENT_SZ;
  // increase size
  inode_increase_size(inode, new_size, root_inode, fs);
  // write dirent
  DirEntry dirent;
  dir_entry_new(&dirent, name, new_inode_id);
  disk_inode_write_at(root_inode, file_count * DIRENT_SZ, (uint8_t *)&dirent,
                      DIRENT_SZ, inode->block_device);
  block_cache_release(bc);

  block_cache_sync_all();

  inode_created->block_id = efs_get_disk_inode_id(fs, new_inode_id);
  inode_created->block_offset = efs_get_disk_inode_offset(fs, new_inode_id);
  inode_created->fs = fs;
  inode_created->block_device = inode->block_device;

  return 0;
}

void inode_ls(Inode *inode, char **v) {
  BlockCache *bc = block_cache_get(inode->block_id, inode->block_device);
  DiskInode *di = (DiskInode *)(bc->cache + inode->block_offset);
  uint64_t file_count = di->size / DIRENT_SZ;
  DirEntry dirent;
  dir_entry_empty(&dirent);
  for (uint64_t i = 0; i < file_count; i++) {
    assert(disk_inode_read_at(di, i * DIRENT_SZ, (uint8_t *)&dirent, DIRENT_SZ,
                              inode->block_device) == DIRENT_SZ);
    strcpy(v[i], dirent.name);
  }
  block_cache_release(bc);
}

uint64_t inode_ls_len(Inode *inode) {
  BlockCache *bc = block_cache_get(inode->block_id, inode->block_device);
  DiskInode *di = (DiskInode *)(bc->cache + inode->block_offset);
  uint64_t file_count = di->size / DIRENT_SZ;
  block_cache_release(bc);
  return file_count;
}

uint64_t inode_read_at(Inode *inode, uint64_t offset, uint8_t *buf,
                       uint64_t len) {
  BlockCache *bc = block_cache_get(inode->block_id, inode->block_device);
  DiskInode *di = (DiskInode *)(bc->cache + inode->block_offset);
  uint64_t ret = disk_inode_read_at(di, offset, buf, len, inode->block_device);
  block_cache_release(bc);
  return ret;
}

uint64_t inode_write_at(Inode *inode, uint64_t offset, uint8_t *buf,
                        uint64_t len) {
  EasyFileSystem *fs = inode->fs;
  BlockCache *bc = block_cache_get(inode->block_id, inode->block_device);
  bc->modified = true;
  DiskInode *di = (DiskInode *)(bc->cache + inode->block_offset);
  inode_increase_size(inode, offset + len, di, fs);
  uint64_t ret = disk_inode_write_at(di, offset, buf, len, inode->block_device);
  block_cache_release(bc);

  block_cache_sync_all();
  return ret;
}

void inode_clear(Inode *inode) {
  EasyFileSystem *fs = inode->fs;
  BlockCache *bc = block_cache_get(inode->block_id, inode->block_device);
  bc->modified = true;
  DiskInode *di = (DiskInode *)(bc->cache + inode->block_offset);
  uint32_t size = di->size;
  uint32_t data_blocks_dealloc_len = disk_inode_data_blocks(di);
  uint32_t *data_blocks_dealloc =
      MALLOC(sizeof(uint32_t) * data_blocks_dealloc_len);
  disk_inode_clear_size(di, data_blocks_dealloc, inode->block_device);
  assert(data_blocks_dealloc_len == disk_inode_total_blocks(size));
  for (uint64_t i = 0; i < data_blocks_dealloc_len; i++) {
    efs_dealloc_data(fs, data_blocks_dealloc[i]);
  }
  FREE(data_blocks_dealloc);
  block_cache_release(bc);

  block_cache_sync_all();
}
