#include <stdint.h>

#include "log.h"
#include "mm.h"
#include "sbi.h"
#include "string.h"
#include "task.h"

int64_t sys_write(uint64_t fd, char *buf, uint64_t len) {
  static uint8_t sys_write_buf[1024];

  switch (fd) {
  case FD_STDOUT:
    copy_byte_buffer(task_current_user_token(), sys_write_buf, (uint8_t *)buf,
                     len, FROM_USER);
    for (uint64_t i = 0; i < len; i++) {
      console_putchar(sys_write_buf[i]);
    }
    return (int64_t)len;
  default:
    error("Unsupported fd %lld in sys_write!\n", fd);
    break;
  }
  return -1;
}
