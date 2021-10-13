#ifndef _SYSCALL_H_
#define _SYSCALL_H_

#include <stdint.h>

#define SYSCALL_WRITE 64
#define SYSCALL_EXIT 93
#define SYSCALL_YIELD 124
#define SYSCALL_GET_TIME 169

int64_t write(uint64_t fd, char *buf, uint64_t len);
int64_t exit(int exit_code);
int64_t yield();
int64_t get_time();

#endif // _SYSCALL_H_
