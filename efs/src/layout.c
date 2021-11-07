#include "efs.h"
#include "log.h"
#include "string.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))

void super_block_initialize(SuperBlock *s, uint32_t total_blocks,
                            uint32_t inode_bitmap_blocks,
                            uint32_t inode_area_blocks,
                            uint32_t data_bitmap_blocks,
                            uint32_t data_area_blocks) {
  s->magic = EFS_MAGIC;
  s->total_blocks = total_blocks;
  s->inode_bitmap_blocks = inode_bitmap_blocks;
  s->inode_area_blocks = inode_area_blocks;
  s->data_bitmap_blocks = data_bitmap_blocks;
  s->data_area_blocks = data_area_blocks;
}

bool super_block_is_valid(SuperBlock *s) { return s->magic == EFS_MAGIC; }

// indirect1 and indirect2 block are allocated only when they are needed
void disk_inode_initialize(DiskInode *d, uint32_t type_) {
  memset(d, 0, sizeof(DiskInode));
  d->type_ = type_;
}

bool disk_inode_is_dir(DiskInode *d) {
  return d->type_ == DISK_INODE_DIRECTORY;
}

bool disk_inode_is_file(DiskInode *d) {
  // not used
  return d->type_ == DISK_INODE_FILE;
}

static uint32_t _disk_inode_data_blocks(uint32_t size) {
  return (size + BLOCK_SZ - 1) / BLOCK_SZ;
}

// Return block number correspond to size
uint32_t disk_inode_data_blocks(DiskInode *d) {
  return _disk_inode_data_blocks(d->size);
}

// Return number of blocks needed include indirect1/2
uint32_t disk_inode_total_blocks(uint32_t size) {
  uint32_t data_blocks = _disk_inode_data_blocks(size);
  uint32_t total = data_blocks;
  // indirect 1
  if (data_blocks > INODE_DIRECT_COUNT) {
    total += 1;
  }
  // indirect2
  if (data_blocks > INDIRECT1_BOUND) {
    total += 1;
    // sub indirect1
    total += (data_blocks - INDIRECT1_BOUND + INODE_INDIRECT1_COUNT - 1) /
             INODE_INDIRECT1_COUNT;
  }
  return total;
}

uint32_t disk_inode_blocks_num_needed(DiskInode *d, uint32_t new_size) {
  assert(new_size > d->size);
  return disk_inode_total_blocks(new_size) - disk_inode_total_blocks(d->size);
}

uint32_t disk_inode_get_block_id(DiskInode *d, uint32_t inner_id,
                                 BlockDevice *device) {
  BlockCache *bc;
  IndirectBlock indirect1;
  IndirectBlock indirect2;
  uint32_t last, t, ret;
  if (inner_id < INODE_DIRECT_COUNT) {
    return d->direct[inner_id];
  } else if (inner_id < INDIRECT1_BOUND) {
    bc = block_cache_get(d->indirect1, device);
    indirect1 = (IndirectBlock)bc->cache;
    ret = indirect1[inner_id - INODE_DIRECT_COUNT];
    block_cache_release(bc);
    return ret;
  } else {
    last = inner_id - INDIRECT1_BOUND;
    bc = block_cache_get(d->indirect2, device);
    indirect2 = (IndirectBlock)bc->cache;
    t = indirect2[last / INODE_INDIRECT1_COUNT];
    block_cache_release(bc);
    bc = block_cache_get(t, device);
    indirect1 = (IndirectBlock)bc->cache;
    ret = indirect1[last % INODE_INDIRECT1_COUNT];
    block_cache_release(bc);
    return ret;
  }
}

void disk_inode_increase_size(DiskInode *d, uint32_t new_size,
                              uint32_t *new_blocks, BlockDevice *device) {
  uint32_t current_blocks = disk_inode_data_blocks(d);
  d->size = new_size;
  uint32_t total_blocks = disk_inode_data_blocks(d);

  // fill direct
  while (current_blocks < MIN(total_blocks, INODE_DIRECT_COUNT)) {
    d->direct[current_blocks] = *new_blocks;
    new_blocks++;
    current_blocks++;
  }

  // alloc indirect1
  if (total_blocks > INODE_DIRECT_COUNT) {
    if (current_blocks == INODE_DIRECT_COUNT) {
      d->indirect1 = *new_blocks;
      new_blocks++;
    }
    current_blocks -= INODE_DIRECT_COUNT;
    total_blocks -= INODE_DIRECT_COUNT;
  } else {
    return;
  }

  BlockCache *bc;
  BlockCache *bc2;

  // fill indirect1
  bc = block_cache_get(d->indirect1, device);
  bc->modified = true;
  IndirectBlock indirect1 = (IndirectBlock)bc->cache;
  while (current_blocks < MIN(total_blocks, INODE_INDIRECT1_COUNT)) {
    indirect1[current_blocks] = *new_blocks;
    new_blocks++;
    current_blocks++;
  }
  block_cache_release(bc);
  // alloc indirect2
  if (total_blocks > INODE_INDIRECT1_COUNT) {
    if (current_blocks == INODE_INDIRECT1_COUNT) {
      d->indirect2 = *new_blocks;
      new_blocks++;
    }
    current_blocks -= INODE_INDIRECT1_COUNT;
    total_blocks -= INODE_INDIRECT1_COUNT;
  } else {
    return;
  }

  // fill indirect2 from (a0, b0) -> (a1, b1)
  uint64_t a0 = current_blocks / INODE_INDIRECT1_COUNT;
  uint64_t b0 = current_blocks % INODE_INDIRECT1_COUNT;
  uint64_t a1 = total_blocks / INODE_INDIRECT1_COUNT;
  uint64_t b1 = total_blocks % INODE_INDIRECT1_COUNT;
  // allow low-level indirect1
  bc2 = block_cache_get(d->indirect2, device);
  bc2->modified = true;
  IndirectBlock indirect2 = (IndirectBlock)bc2->cache;
  while (a0 < a1 || (a0 == a1 && b0 < b1)) {
    if (b0 == 0) {
      indirect2[a0] = *new_blocks;
      new_blocks++;
    }
    // fill current
    bc = block_cache_get(indirect2[a0], device);
    bc->modified = true;
    indirect1 = (IndirectBlock)bc->cache;
    indirect1[b0] = *new_blocks;
    new_blocks++;
    block_cache_release(bc);
    // move to next
    b0 += 1;
    if (b0 == INODE_INDIRECT1_COUNT) {
      b0 = 0;
      a0 += 1;
    }
  }
  block_cache_release(bc2);
}

// Clear size to zero and return blocks that should be deallocated
// We will clear the block contents to zero later
void disk_inode_clear_size(DiskInode *d, uint32_t *v, BlockDevice *device) {
  uint64_t data_blocks = disk_inode_data_blocks(d);
  d->size = 0;
  uint64_t current_blocks = 0;
  BlockCache *bc;
  BlockCache *bc2;
  IndirectBlock indirect1;
  IndirectBlock indirect2;

  // direct
  while (current_blocks < MIN(data_blocks, INODE_DIRECT_COUNT)) {
    *v = d->direct[current_blocks];
    v++;
    d->direct[current_blocks] = 0;
    current_blocks += 1;
  }

  // indirect1 block
  if (data_blocks > INODE_DIRECT_COUNT) {
    *v = d->indirect1;
    v++;
    data_blocks -= INODE_DIRECT_COUNT;
    current_blocks = 0;
  } else {
    return;
  }

  // indirect1
  bc = block_cache_get(d->indirect1, device);
  bc->modified = true;
  indirect1 = (IndirectBlock)bc->cache;
  while (current_blocks < MIN(data_blocks, INODE_INDIRECT1_COUNT)) {
    *v = indirect1[current_blocks];
    v++;
    current_blocks += 1;
  }
  block_cache_release(bc);
  d->indirect1 = 0;

  // indirect2 block
  if (data_blocks > INODE_INDIRECT1_COUNT) {
    *v = d->indirect2;
    v++;
    data_blocks -= INODE_INDIRECT1_COUNT;
  } else {
    return;
  }

  // indirect2
  assert(data_blocks <= INODE_INDIRECT2_COUNT);
  uint64_t a1 = data_blocks / INODE_INDIRECT1_COUNT;
  uint64_t b1 = data_blocks % INODE_INDIRECT1_COUNT;
  bc2 = block_cache_get(d->indirect1, device);
  bc2->modified = true;
  indirect2 = (IndirectBlock)bc2->cache;
  // full indirect1 blocks
  for (uint64_t i = 0; i < a1; i++) {
    *v = indirect2[i];
    v++;
    bc = block_cache_get(indirect2[i], device);
    bc->modified = true;
    indirect1 = (IndirectBlock)bc->cache;
    for (uint64_t j = 0; j < INODE_INDIRECT1_COUNT; j++) {
      *v = indirect1[j];
      v++;
    }
    block_cache_release(bc);
  }
  // last indirect1 block
  if (b1 > 0) {
    *v = indirect2[a1];
    v++;
    bc = block_cache_get(indirect2[a1], device);
    bc->modified = true;
    indirect1 = (IndirectBlock)bc->cache;
    for (uint64_t j = 0; j < b1; j++) {
      *v = indirect1[j];
      v++;
    }
    block_cache_release(bc);
  }
  block_cache_release(bc2);
  d->indirect2 = 0;
}

uint64_t disk_inode_read_at(DiskInode *d, uint64_t offset, uint8_t *buf,
                            uint64_t len, BlockDevice *device) {
  uint64_t start = offset;
  uint64_t end = MIN(d->size, offset + len);
  if (start >= end) {
    return 0;
  }
  uint64_t start_block = start / BLOCK_SZ;
  uint64_t read_size = 0;
  uint64_t end_current_block, block_read_size;
  uint8_t *dst;
  uint8_t *src;
  BlockCache *bc;
  while (1) {
    // calculate end of current block
    end_current_block = MIN((start / BLOCK_SZ + 1) * BLOCK_SZ, end);
    // read and update read size
    block_read_size = end_current_block - start;
    bc = block_cache_get(disk_inode_get_block_id(d, start_block, device),
                         device);
    dst = buf + read_size;
    src = &bc->cache[start % BLOCK_SZ];
    memcpy(dst, src, block_read_size);
    block_cache_release(bc);
    read_size += block_read_size;
    // move to next block
    if (end_current_block == end) {
      break;
    }
    start_block += 1;
    start = end_current_block;
  }
  return read_size;
}

// File size must be adjusted before
uint64_t disk_inode_write_at(DiskInode *d, uint64_t offset, uint8_t *buf,
                             uint64_t len, BlockDevice *device) {
  uint64_t start = offset;
  uint64_t end = MIN(d->size, offset + len);
  assert(start <= end);
  uint64_t start_block = start / BLOCK_SZ;
  uint64_t write_size = 0;
  uint64_t end_current_block, block_write_size;
  uint8_t *dst;
  uint8_t *src;
  BlockCache *bc;
  while (1) {
    // calculate end of current block
    end_current_block = MIN((start / BLOCK_SZ + 1) * BLOCK_SZ, end);
    // write and update write size
    block_write_size = end_current_block - start;
    bc = block_cache_get(disk_inode_get_block_id(d, start_block, device),
                         device);
    bc->modified = true;
    src = buf + write_size;
    dst = &bc->cache[start % BLOCK_SZ];
    memcpy(dst, src, block_write_size);
    block_cache_release(bc);
    write_size += block_write_size;
    // move to next block
    if (end_current_block == end) {
      break;
    }
    start_block += 1;
    start = end_current_block;
  }
  return write_size;
}

void dir_entry_empty(DirEntry *e) {
  memset(e->name, 0, NAME_LENGTH_LIMIT + 1);
  e->inode_number = 0;
}

void dir_entry_new(DirEntry *e, char *name, uint32_t inode_number) {
  memcpy(e->name, name, NAME_LENGTH_LIMIT + 1);
  e->inode_number = inode_number;
}
