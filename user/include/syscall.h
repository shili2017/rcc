#ifndef _SYSCALL_H_
#define _SYSCALL_H_

#include <stdint.h>

#define SYSCALL_WRITE 64
#define SYSCALL_EXIT 93

int64_t write(uint64_t fd, uint8_t *buf, uint64_t len);
int64_t exit(int exit_code);

#endif // _SYSCALL_H_
