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
    copy_byte_buffer(processor_current_user_token(), sys_write_buf,
                     (uint8_t *)buf, len, FROM_USER);
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

int64_t sys_read(uint64_t fd, char *buf, uint64_t len) {
  uint64_t c;
  uint8_t ch;
  switch (fd) {
  case FD_STDIN:
    assert(len != 1, "Only support len = 1 in sys_read!\n");
    while (1) {
      c = console_getchar();
      if (c == 0) {
        task_suspend_current_and_run_next();
        continue;
      } else {
        break;
      }
    }
    ch = (uint8_t)c;
    copy_byte_buffer(processor_current_user_token(), &ch, (uint8_t *)buf, 1,
                     TO_USER);
    return 1;
  default:
    error("Unsupported fd %lld in sys_read!\n", fd);
    break;
  }
  return -1;
}
