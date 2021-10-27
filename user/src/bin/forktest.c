#include <stdint.h>

#include "assert.h"
#include "stdio.h"
#include "syscall.h"

#define MAX_CHILD 40

int main() {
  int64_t pid;
  int exit_code = 0;
  for (uint64_t i = 0; i < MAX_CHILD; i++) {
    pid = fork();
    if (pid == 0) {
      printf("I am child %lld with pid=%lld\n", i, getpid());
      exit(0);
    } else {
      printf("forked child pid = %lld\n", pid);
    }
    assert(pid > 0);
  }
  for (uint64_t i = 0; i < MAX_CHILD; i++) {
    if (wait(&exit_code) <= 0) {
      printf("error: wait stopped early 1\n");
      exit(-1);
    }
  }
  if (exit_code > 0) {
    printf("error: wait stopped early 2\n");
    exit(-1);
  }
  printf("forktest pass.\n");
  return 0;
}
