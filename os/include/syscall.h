#ifndef _SYSCALL_H_
#define _SYSCALL_H_

#include <stdint.h>

#include "timer.h"

#define SYSCALL_WRITE 64
#define SYSCALL_EXIT 93
#define SYSCALL_YIELD 124
#define SYSCALL_SET_PRIORITY 140
#define SYSCALL_GET_TIME 169
#define SYSCALL_MUNMAP 215
#define SYSCALL_MMAP 222

int64_t syscall(uint64_t syscall_id, uint64_t a0, uint64_t a1, uint64_t a2);

int64_t sys_write(uint64_t fd, char *buf, uint64_t len);
int64_t sys_exit(int exit_code);
int64_t sys_yield();
int64_t sys_set_priority(int64_t prio);
int64_t sys_get_time(TimeVal *ts, int64_t tz);
int64_t sys_mmap(uint64_t start, uint64_t len, uint64_t prot);
int64_t sys_munmap(uint64_t start, uint64_t len);

#endif // _SYSCALL_H_
