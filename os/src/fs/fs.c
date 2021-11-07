#include "fs.h"
#include "log.h"
#include "sbi.h"
#include "string.h"
#include "task.h"

int64_t stdin_read(char *buf, uint64_t len) {
  assert(len == 1);

  // busy loop
  uint64_t c;
  while (1) {
    c = console_getchar();
    if (c == 0) {
      task_suspend_current_and_run_next();
      continue;
    } else {
      break;
    }
  }

  uint8_t ch = (uint8_t)c;
  copy_byte_buffer(processor_current_user_token(), &ch, (uint8_t *)buf, 1,
                   TO_USER);
  return 1;
}

int64_t stdout_write(char *buf, uint64_t len) {
  static uint8_t stdout_write_buf[1024];

  copy_byte_buffer(processor_current_user_token(), stdout_write_buf,
                   (uint8_t *)buf, len, FROM_USER);
  for (uint64_t i = 0; i < len; i++) {
    console_putchar((char)stdout_write_buf[i]);
  }
  return (int64_t)len;
}
