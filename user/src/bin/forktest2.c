#include <stdint.h>

#include "assert.h"
#include "stdio.h"
#include "syscall.h"

#define NUM 30

int main() {
  int64_t pid;
  int exit_code = 0;
  int64_t current_time, sleep_length;
  for (uint64_t i = 0; i < NUM; i++) {
    pid = fork();
    if (pid == 0) {
      current_time = get_time();
      sleep_length = current_time * current_time % 1000 + 1000;
      printf("pid %lld sleep for %lld ms\n", getpid(), sleep_length);
      sleep((uint64_t)sleep_length);
      printf("pid %lld OK!\n", getpid());
      exit(0);
    }
  }
  for (uint64_t i = 0; i < NUM; i++) {
    assert(wait(&exit_code) > 0);
    assert(exit_code == 0);
  }
  assert(wait(&exit_code) < 0);
  printf("forktest2 test passed!\n");
  return 0;
}
