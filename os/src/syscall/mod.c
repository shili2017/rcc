#include "log.h"
#include "syscall.h"

int64_t syscall(uint64_t syscall_id, uint64_t a0, uint64_t a1, uint64_t a2) {
  switch (syscall_id) {
  case SYSCALL_WRITE:
    return sys_write(a0, (char *)a1, a2);
  case SYSCALL_EXIT:
    return sys_exit((int)a0);
  default:
    panic("Unsupported syscall_id: %lld", syscall_id);
    break;
  }
  return -1;
}
