#include "syscall.h"
#include "log.h"
#include "timer.h"

int64_t syscall(uint64_t syscall_id, uint64_t a0, uint64_t a1, uint64_t a2) {
  switch (syscall_id) {
  case SYSCALL_DUP:
    return sys_dup(a0);
  case SYSCALL_OPEN:
    return sys_open((char *)a0, (uint32_t)a1);
  case SYSCALL_CLOSE:
    return sys_close(a0);
  case SYSCALL_PIPE:
    return sys_pipe((uint64_t *)a0);
  case SYSCALL_READ:
    return sys_read(a0, (char *)a1, a2);
  case SYSCALL_WRITE:
    return sys_write(a0, (char *)a1, a2);
  case SYSCALL_EXIT:
    return sys_exit((int)a0);
  case SYSCALL_YIELD:
    return sys_yield();
  case SYSCALL_SET_PRIORITY:
    return sys_set_priority((int64_t)a0);
  case SYSCALL_GET_TIME:
    return sys_get_time((TimeVal *)a0, (int64_t)a1);
  case SYSCALL_GETPID:
    return sys_getpid();
  case SYSCALL_MUNMAP:
    return sys_munmap(a0, a1);
  case SYSCALL_FORK:
    return sys_fork();
  case SYSCALL_EXEC:
    return sys_exec((char *)a0);
  case SYSCALL_MMAP:
    return sys_mmap(a0, a1, a2);
  case SYSCALL_WAITPID:
    return sys_waitpid((int64_t)a0, (int *)a1);
  case SYSCALL_SPAWN:
    return sys_spawn((char *)a0);
  case SYSCALL_MAILREAD:
    return sys_mailread((char *)a0, a1);
  case SYSCALL_MAILWRITE:
    return sys_mailwrite((int64_t)a0, (char *)a1, a2);
  default:
    panic("Unsupported syscall_id: %lld\n", syscall_id);
    break;
  }
  return -1;
}
