#include <stdint.h>

#include "stdio.h"
#include "string.h"
#include "syscall.h"

#define LINE_LENGTH 256

#define LF (0x0a)
#define CR (0x0d)
#define DL (0x7f)
#define BS (0x08)

int main() {
  static char line[LINE_LENGTH];
  char c;
  int64_t i = 0, pid, exit_pid;
  int exit_code;

  memset(line, 0, LINE_LENGTH);
  printf("rcc user shell\n");
  printf(">> ");

  while (1) {
    c = getchar();
    switch (c) {
    case LF:
    case CR:
      printf("\n");
      if (i > 0) {
        line[i] = '\0';
        i++;
        pid = fork();
        if (pid == 0) {
          // child process
          if (exec(line) == -1) {
            printf("Error when executing!\n");
            return -4;
          }
          printf("Error: unreachable\n");
        } else {
          exit_code = 0;
          exit_pid = waitpid((uint64_t)pid, &exit_code);
          if (pid != exit_pid) {
            printf("Error: pid (%lld) != exit_pid (%lld)\n", pid, exit_pid);
          }
          printf("Shell: Process %lld exited with code %lld\n", pid, exit_code);
        }
        memset(line, 0, LINE_LENGTH);
        i = 0;
      }
      printf(">> ");
      break;
    case BS:
    case DL:
      if (i > 0) {
        printf("%c", (char)BS);
        printf(" ");
        printf("%c", (char)BS);
        line[i] = '\0';
        i--;
      }
      break;
    default:
      printf("%c", c);
      line[i] = c;
      i++;
      break;
    }
  }

  return 0;
}
