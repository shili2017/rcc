#ifndef _CONFIG_H_
#define _CONFIG_H_

#define USER_STACK_SIZE (4096 * 1)
#define KERNEL_STACK_SIZE (4096 * 2)
#define MAX_APP_NUM 16
#define APP_BASE_ADDRESS 0x80400000
#define APP_SIZE_LIMIT 0x20000

// clock freq for qemu
#define CLOCK_FREQ 10000000

#endif // _CONFIG_H_
