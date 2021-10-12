#ifndef _SYSCALL_H_
#define _SYSCALL_H_

#include <stdint.h>

#define SYSCALL_WRITE 64
#define SYSCALL_EXIT 93

int64_t syscall(uint64_t syscall_id, uint64_t a0, uint64_t a1, uint64_t a2);

int64_t sys_write(uint64_t fd, uint8_t *buf, uint64_t len);
int64_t sys_exit(int exit_code);

#endif // _SYSCALL_H_
