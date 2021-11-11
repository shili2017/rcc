#ifndef _SYSCALL_H_
#define _SYSCALL_H_

#include <stdint.h>

#define SYSCALL_DUP 24
#define SYSCALL_OPEN 56
#define SYSCALL_CLOSE 57
#define SYSCALL_PIPE 59
#define SYSCALL_READ 63
#define SYSCALL_WRITE 64
#define SYSCALL_EXIT 93
#define SYSCALL_YIELD 124
#define SYSCALL_SET_PRIORITY 140
#define SYSCALL_GET_TIME 169
#define SYSCALL_GETPID 172
#define SYSCALL_MUNMAP 215
#define SYSCALL_FORK 220
#define SYSCALL_EXEC 221
#define SYSCALL_MMAP 222
#define SYSCALL_WAITPID 260
#define SYSCALL_SPAWN 400
#define SYSCALL_MAILREAD 401
#define SYSCALL_MAILWRITE 402

int64_t dup(uint64_t fd);
int64_t open(char *path, uint32_t flags);
int64_t close(uint64_t fd);
int64_t pipe(uint64_t *pipe);
int64_t read(uint64_t rd, char *buf, uint64_t len);
int64_t write(uint64_t fd, char *buf, uint64_t len);
int64_t exit(int exit_code);
int64_t yield();
int64_t set_priority(int64_t prio);
int64_t get_time();
int64_t sleep(uint64_t period_ms);
int64_t getpid();
int64_t munmap(uint64_t start, uint64_t len);
int64_t fork();
int64_t exec(char *path);
int64_t mmap(uint64_t start, uint64_t len, uint64_t prot);
int64_t wait(int *exit_code);
int64_t waitpid(int64_t pid, int *exit_code);
int64_t spawn(char *path);
int64_t mailread(char *buf, uint64_t len);
int64_t mailwrite(int64_t pid, char *buf, uint64_t len);

#endif // _SYSCALL_H_
