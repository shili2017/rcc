#include <stdint.h>

#include "stdio.h"
#include "syscall.h"

int main() {
  int64_t f = fork();
  int exit_code = 0;
  int64_t pid;

  if (f == 0) {
    exec("user_shell\0");
  } else {
    while (1) {
      pid = wait(&exit_code);
      if (pid == -1) {
        yield();
        continue;
      }
      printf("[initproc] Released a zombie process, pid=%lld, exit_code=%d\n",
             pid, exit_code);
    }
  }

  return 0;
}
