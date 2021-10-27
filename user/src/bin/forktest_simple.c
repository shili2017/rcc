#include <stdint.h>

#include "assert.h"
#include "stdio.h"
#include "syscall.h"

int main() {
  assert(wait(NULL) == -1);

  printf("sys_wait without child process test passed!\n");
  printf("parent start, pid = %lld!\n", getpid());

  int64_t pid = fork();
  int exit_code = 0;
  if (pid == 0) {
    // child process
    printf("hello child process!\n");
    return 100;
  } else {
    // parent process
    printf("ready waiting on parent process!\n");
    assert(pid == wait(&exit_code));
    assert(exit_code == 100);
    printf("child process pid = %lld, exit code = %lld\n", pid, exit_code);
    return 0;
  }
}
