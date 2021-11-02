#include <stdint.h>

#include "assert.h"
#include "stdio.h"
#include "string.h"
#include "syscall.h"

#define BUF_LEN 256

int main() {
  int64_t pid = getpid();

  char buffer0[27];
  memset(buffer0, 'a', 27);
  assert(mailwrite(pid, buffer0, 27) == 27);

  char buffer1[BUF_LEN + 1];
  memset(buffer1, 'b', BUF_LEN + 1);
  assert(mailwrite(pid, buffer1, BUF_LEN + 1) == BUF_LEN);

  char buf[BUF_LEN];
  memset(buf, 0, BUF_LEN);
  assert(mailread(buf, 27) == 27);
  assert(strncmp(buffer0, buf, 27) == 0);

  assert(mailread(buf, 27) == 27);
  assert(strncmp(buffer1, buf, 27) == 0);

  printf("mailtest passed!\n");
  return 0;
}
