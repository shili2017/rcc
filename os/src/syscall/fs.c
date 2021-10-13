#include <stdint.h>

#include "batch.h"
#include "log.h"
#include "stdio.h"
#include "string.h"

int sys_write_check(char *buf, uint64_t len) {
  uint64_t start = (uint64_t)buf;
  uint64_t end = start + len;
  return check_address_range(start, end);
}

int64_t sys_write(uint64_t fd, char *buf, uint64_t len) {
  switch (fd) {
  case FD_STDOUT:
    if (sys_write_check(buf, len)) {
      for (uint64_t i = 0; i < len; i++) {
        console_putchar(buf[i]);
      }
      return (int64_t)len;
    } else {
      error("Invalid memory address access 0x%llx in sys_write!\n",
            (uint64_t)buf);
      return -1;
    }
  default:
    error("Unsupported fd %lld in sys_write!\n", fd);
    break;
  }
  return -1;
}
