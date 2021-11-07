#include "drivers.h"
#include "virtio.h"

const int R = 0;
const int W = 1;

void virtio_read_block(BlockCache *b) { virtio_disk_rw(b, R); }

void virtio_write_block(BlockCache *b) { virtio_disk_rw(b, W); }
