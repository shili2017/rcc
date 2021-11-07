#include "efs.h"
#include "log.h"

#define BLOCK_POS(bit) ((bit) / BLOCK_BITS)
#define BITS64_POS(bit) (((bit) % BLOCK_BITS) / 64)
#define INNER_POS(bit) (((bit) % BLOCK_BITS) % 64)

void bitmap_new(Bitmap *b, uint64_t start_block_id, uint64_t blocks) {
  b->start_block_id = start_block_id;
  b->blocks = blocks;
}

static uint64_t _ctzll(uint64_t x) {
  uint64_t ret = 0;
  for (uint64_t i = 0; i < 64; i++) {
    if ((x & (1LL << i)) == 0) {
      ret++;
    } else {
      break;
    }
  }
  return ret;
}

// return UINT64_MAX (-1) when alloc fails
uint64_t bitmap_alloc(Bitmap *b, BlockDevice *device) {
  BlockCache *bc;
  BitmapBlock *bitmap_block;
  uint64_t block_id = 0, bits64_pos = 0, inner_pos = 0;
  for (block_id = 0; block_id < b->blocks; block_id++) {
    bc = block_cache_get(block_id + b->start_block_id, device);
    bc->modified = true;
    bitmap_block = (BitmapBlock *)bc->cache;
    for (bits64_pos = 0; bits64_pos < 64; bits64_pos++) {
      if (bitmap_block[bits64_pos] != UINT64_MAX) {
        inner_pos = _ctzll(~bitmap_block[bits64_pos]);
        // modify cache
        bitmap_block[bits64_pos] |= (1LL << inner_pos);
        block_cache_release(bc);
        return block_id * BLOCK_BITS + bits64_pos * 64 + inner_pos;
      }
    }
    block_cache_release(bc);
  }
  return UINT64_MAX;
}

void bitmap_dealloc(Bitmap *b, BlockDevice *device, uint64_t bit) {
  uint64_t block_pos = BLOCK_POS(bit);
  uint64_t bits64_pos = BITS64_POS(bit);
  uint64_t inner_pos = INNER_POS(bit);
  BlockCache *bc = block_cache_get(block_pos + b->start_block_id, device);
  bc->modified = true;
  BitmapBlock *bitmap_block = (BitmapBlock *)bc->cache;
  assert((bitmap_block[bits64_pos] & (1LL << inner_pos)) != 0);
  bitmap_block[bits64_pos] -= (1LL << inner_pos);
  block_cache_release(bc);
}

uint64_t bitmap_maximum(Bitmap *b) { return b->blocks * BLOCK_BITS; }
