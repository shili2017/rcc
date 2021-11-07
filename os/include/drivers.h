#ifndef _DRIVERS_H_
#define _DRIVERS_H_

#include "efs.h"

// virtio_disk.c
void virtio_disk_init(void);
void virtio_disk_rw(BlockCache *, int);
void virtio_disk_intr(void);

// virtio_blk.c
void virtio_read_block(BlockCache *);
void virtio_write_block(BlockCache *);

#endif // _DRIVERS_H_
