#include <stdint.h>

#include "assert.h"
#include "stdio.h"
#include "string.h"
#include "syscall.h"

static char STR[] = "Pipetest!";

int main() {
  uint64_t pipe_fd[2];
  pipe(pipe_fd);
  printf("[parent] read end = %lld, write end = %lld\n", pipe_fd[0],
         pipe_fd[1]);

  if (fork() == 0) {
    // child process, read from parent
    close(pipe_fd[1]); // close write_end
    char buffer[32 + 1];
    int64_t len_read = read(pipe_fd[0], buffer, 32);
    buffer[len_read] = 0;
    close(pipe_fd[0]); // close read end

    printf("[child ] read len = %lld, string = %s\n", len_read, buffer);
    printf("[child ] Read OK, child process exited!\n");

    return 0;
  } else {
    // parent process, write to child
    close(pipe_fd[0]); // close read end
    int64_t len_write = write(pipe_fd[1], STR, strlen(STR));
    printf("[parent] write len = %lld, string = %s\n", len_write, STR);
    close(pipe_fd[1]); // close write end

    int exit_code = 0;
    wait(&exit_code);
    assert(exit_code == 0);
    printf("pipetest passed!\n");
  }
  return 0;
}
