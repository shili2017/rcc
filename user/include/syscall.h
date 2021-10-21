#ifndef _SYSCALL_H_
#define _SYSCALL_H_

#include <stdint.h>

#define SYSCALL_WRITE 64
#define SYSCALL_EXIT 93
#define SYSCALL_YIELD 124
#define SYSCALL_SET_PRIORITY 140
#define SYSCALL_GET_TIME 169
#define SYSCALL_MUNMAP 215
#define SYSCALL_MMAP 222

int64_t write(uint64_t fd, char *buf, uint64_t len);
int64_t exit(int exit_code);
int64_t yield();
int64_t set_priority(int64_t prio);
int64_t get_time();
int64_t mmap(uint64_t start, uint64_t len, uint64_t prot);
int64_t munmap(uint64_t start, uint64_t len);

#endif // _SYSCALL_H_
