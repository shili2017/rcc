#include <stdint.h>

#include "stdio.h"
#include "syscall.h"

// putchar function to console
inline void _putchar(char character) {
  write(FD_STDOUT, (uint8_t *)&character, 1);
}

extern uint8_t start_bss, end_bss;

void clear_bss() {
  for (uint8_t *i = &start_bss; i < &end_bss; i++) {
    *i = 0;
  }
}

int main();

__attribute__((section(".text.entry"))) void _start() {
  clear_bss();
  exit(main());
  printf("unreachable after sys_exit!\n");
}
