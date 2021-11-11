#include <stdint.h>

#include "assert.h"
#include "fcntl.h"
#include "stdio.h"
#include "string.h"
#include "syscall.h"

int main() {
  int exit_code;
  int fd = open("test\0", O_CREATE | O_WRONLY);
  printf("open OK, fd = %d\n", fd);
  char str[100] = "hello world!\0";
  int len = strlen(str);
  write(fd, str, len);
  close(fd);
  printf("write over.\n");
  if (fork() == 0) {
    int fd = open("test\0", O_RDONLY);
    char str[100];
    str[read(fd, str, len)] = 0;
    printf("%s\n", str);
    printf("read over.\n");
    close(fd);
    exit(0);
  }
  wait(&exit_code);
  printf("filetest passed.\n");
  return 0;
}
