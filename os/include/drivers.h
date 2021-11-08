#ifndef _DRIVERS_H_
#define _DRIVERS_H_

#include "efs.h"

// virtio_disk.c
void virtio_disk_init();
void virtio_disk_rw(BlockCache *, int);
void virtio_disk_intr();

// virtio_blk.c
void virtio_read_block(BlockCache *);
void virtio_write_block(BlockCache *);
BlockDevice *virtio_block_device_init();

// plic.c
void plic_init();
int plic_claim();
void plic_complete(int);

#endif // _DRIVERS_H_
