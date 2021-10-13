#include <stdint.h>

#include "log.h"
#include "stdio.h"
#include "string.h"

int64_t sys_write(uint64_t fd, char *buf, uint64_t len) {
  switch (fd) {
  case FD_STDOUT:
    for (uint64_t i = 0; i < len; i++) {
      console_putchar(buf[i]);
    }
    return (int64_t)len;
  default:
    info("Unsupported fd %lld in sys_write!\n", fd);
    break;
  }
  return -1;
}
