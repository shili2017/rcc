#include <stdint.h>

#include "fs.h"
#include "log.h"
#include "printf.h"

int64_t sys_write(uint64_t fd, uint8_t *buf, uint64_t len) {
  switch (fd) {
  case FD_STDOUT:
    printf((char *)buf);
    return (int64_t)len;
  default:
    panic("Unsupported fd in sys_write!\n");
    break;
  }
  return -1;
}
