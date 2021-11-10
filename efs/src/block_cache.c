#include <stddef.h>

#include "efs.h"
#include "log.h"
#include "string.h"

static void block_cache_new(BlockCache *block_cache, uint64_t block_id,
                            BlockDevice *block_device) {
  memset(block_cache->cache, 0, BLOCK_SZ);
  block_cache->block_id = block_id;
  block_cache->block_device = block_device;
  block_cache->modified = false;
  block_cache->block_device->read_block(block_cache);
}

static void block_cache_sync(BlockCache *block_cache) {
  if (block_cache->modified) {
    block_cache->modified = false;
    block_cache->block_device->write_block(block_cache);
  }
}

#define BLOCK_CACHE_SIZE 16

typedef struct {
  BlockCache queue[BLOCK_CACHE_SIZE];
  BlockCache head;
} BlockCacheManager;

static BlockCacheManager BLOCK_CACHE_MANAGER;

void block_cache_manager_init() {
  memset(&BLOCK_CACHE_MANAGER, 0, sizeof(BlockCacheManager));
  // Create linked list of buffers
  BLOCK_CACHE_MANAGER.head.prev = &BLOCK_CACHE_MANAGER.head;
  BLOCK_CACHE_MANAGER.head.next = &BLOCK_CACHE_MANAGER.head;
  for (BlockCache *b = BLOCK_CACHE_MANAGER.queue;
       b < BLOCK_CACHE_MANAGER.queue + BLOCK_CACHE_SIZE; b++) {
    b->next = BLOCK_CACHE_MANAGER.head.next;
    b->prev = &BLOCK_CACHE_MANAGER.head;
    BLOCK_CACHE_MANAGER.head.next->prev = b;
    BLOCK_CACHE_MANAGER.head.next = b;
  }
}

BlockCache *block_cache_get(uint64_t block_id, BlockDevice *block_device) {
  // Is the block already cached?
  for (BlockCache *b = BLOCK_CACHE_MANAGER.head.next;
       b != &BLOCK_CACHE_MANAGER.head; b = b->next) {
    if (b->block_id == block_id && b->block_device == block_device) {
      b->ref++;
      return b;
    }
  }
  // Not cached, recycle the least recently used (LRU) unused buffer
  for (BlockCache *b = BLOCK_CACHE_MANAGER.head.prev;
       b != &BLOCK_CACHE_MANAGER.head; b = b->prev) {
    if (b->ref == 0) {
      block_cache_sync(b);
      block_cache_new(b, block_id, block_device);
      b->ref = 1;
      return b;
    }
  }
  panic("Run out of BlockCache!\n");
  return NULL;
}

void block_cache_release(BlockCache *b) {
  b->ref--;
  if (b->ref == 0) {
    // no one is waiting for it.
    b->next->prev = b->prev;
    b->prev->next = b->next;
    b->next = BLOCK_CACHE_MANAGER.head.next;
    b->prev = &BLOCK_CACHE_MANAGER.head;
    BLOCK_CACHE_MANAGER.head.next->prev = b;
    BLOCK_CACHE_MANAGER.head.next = b;
  }
}

void block_cache_sync_all() {
  for (BlockCache *b = BLOCK_CACHE_MANAGER.head.next;
       b != &BLOCK_CACHE_MANAGER.head; b = b->next) {
    block_cache_sync(b);
  }
}
