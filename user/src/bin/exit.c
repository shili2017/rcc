#include <stdint.h>

#include "assert.h"
#include "stdio.h"
#include "syscall.h"

const int MAGIC = -0x10384;

int main() {
  printf("I am the parent. Forking the child...\n");
  int64_t pid = fork();
  if (pid == 0) {
    printf("I am the child.\n");
    for (int64_t i = 0; i < 7; i++) {
      yield();
    }
    exit(MAGIC);
  } else {
    printf("I am parent, fork a child pid %lld\n", pid);
  }
  printf("I am the parent, waiting now..\n");
  int xstate = 0;
  assert(waitpid(pid, &xstate) == pid && xstate == MAGIC);
  assert(waitpid(pid, &xstate) < 0 && wait(&xstate) <= 0);

  printf("waitpid %lld ok.\n", pid);
  printf("exit pass.\n");
  return 0;
}
