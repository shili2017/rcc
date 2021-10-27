#include <stdint.h>

#include "stdio.h"
#include "syscall.h"

void sleepy() {
  uint64_t time = 100;
  for (uint64_t i = 0; i < 5; i++) {
    sleep(time);
    printf("sleep %lld x %lld msecs.\n", i + 1, time);
  }
  exit(0);
}

int main() {
  int64_t start_time = get_time();
  int64_t pid = fork();
  int exit_code = 0;
  if (pid == 0) {
    sleepy();
  }
  if (waitpid(pid, &exit_code) == pid && exit_code == 0) {
    printf("use %lld msecs.\n", get_time() - start_time);
    printf("sleep pass.\n");
  } else {
    printf("sleep fail.\n");
  }
  return 0;
}
