#include <stdint.h>

#include "log.h"
#include "stdio.h"

int64_t sys_write(uint64_t fd, uint8_t *buf, uint64_t len) {
  switch (fd) {
  case FD_STDOUT:
    for (uint64_t i = 0; i < len; i++) {
      printf("%c", buf[i]);
    }
    return (int64_t)len;
  default:
    panic("Unsupported fd in sys_write!\n");
    break;
  }
  return -1;
}
