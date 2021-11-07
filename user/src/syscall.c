#include "syscall.h"
#include "time.h"

static inline int64_t syscall(uint64_t id, uint64_t a0, uint64_t a1,
                              uint64_t a2) {
  int64_t ret;
  asm volatile("mv x10, %1\n"
               "mv x11, %2\n"
               "mv x12, %3\n"
               "mv x17, %4\n"
               "ecall\n"
               "mv %0, x10\n"
               : "=r"(ret)
               : "r"(a0), "r"(a1), "r"(a2), "r"(id)
               : "memory", "x10", "x11", "x12", "x17");
  return ret;
}

int64_t dup(uint64_t fd) { return syscall(SYSCALL_DUP, fd, 0, 0); }

int64_t open(char *path, uint32_t flags) {
  return syscall(SYSCALL_OPEN, (uint64_t)path, (uint64_t)flags, 0);
}

int64_t close(uint64_t fd) { return syscall(SYSCALL_CLOSE, fd, 0, 0); }

int64_t pipe(uint64_t *pipe) {
  return syscall(SYSCALL_PIPE, (uint64_t)pipe, 0, 0);
}

int64_t read(uint64_t fd, char *buf, uint64_t len) {
  return syscall(SYSCALL_READ, fd, (uint64_t)buf, len);
}

int64_t write(uint64_t fd, char *buf, uint64_t len) {
  return syscall(SYSCALL_WRITE, fd, (uint64_t)buf, len);
}

int64_t exit(int exit_code) {
  return syscall(SYSCALL_EXIT, (uint64_t)exit_code, 0, 0);
}

int64_t yield() { return syscall(SYSCALL_YIELD, 0, 0, 0); }

int64_t set_priority(int64_t prio) {
  return syscall(SYSCALL_SET_PRIORITY, (uint64_t)prio, 0, 0);
}

int64_t get_time() {
  TimeVal time;
  if (syscall(SYSCALL_GET_TIME, (uint64_t)&time, 0, 0) == 0) {
    return (int64_t)((time.sec & 0xffff) * 1000 + time.usec / 1000);
  }
  return -1;
}

int64_t sleep(uint64_t period_ms) {
  int64_t start = get_time();
  while (get_time() < start + period_ms) {
    yield();
  }
  return 0;
}

int64_t getpid() { return syscall(SYSCALL_GETPID, 0, 0, 0); }

int64_t munmap(uint64_t start, uint64_t len) {
  return syscall(SYSCALL_MUNMAP, start, len, 0);
}

int64_t fork() { return syscall(SYSCALL_FORK, 0, 0, 0); }

int64_t exec(char *path) { return syscall(SYSCALL_EXEC, (uint64_t)path, 0, 0); }

int64_t mmap(uint64_t start, uint64_t len, uint64_t prot) {
  return syscall(SYSCALL_MMAP, start, len, prot);
}

int64_t wait(int *exit_code) {
  int64_t exit_pid;
  while (1) {
    exit_pid = syscall(SYSCALL_WAITPID, -1, (uint64_t)exit_code, 0);
    if (exit_pid == -2) {
      yield();
    } else {
      return exit_pid;
    }
  }
}

int64_t waitpid(int64_t pid, int *exit_code) {
  int64_t exit_pid;
  while (1) {
    exit_pid = syscall(SYSCALL_WAITPID, (uint64_t)pid, (uint64_t)exit_code, 0);
    if (exit_pid == -2) {
      yield();
    } else {
      return exit_pid;
    }
  }
}

int64_t spawn(char *path) {
  return syscall(SYSCALL_SPAWN, (uint64_t)path, 0, 0);
}

int64_t mailread(char *buf, uint64_t len) {
  return syscall(SYSCALL_MAILREAD, (uint64_t)buf, len, 0);
}

int64_t mailwrite(int64_t pid, char *buf, uint64_t len) {
  return syscall(SYSCALL_MAILWRITE, (uint64_t)pid, (uint64_t)buf, len);
}
