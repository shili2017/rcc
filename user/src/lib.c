#include <stdint.h>

#include "stdio.h"
#include "syscall.h"

int main();

__attribute__((section(".text.entry"))) void _start() {
  exit(main());
  printf("unreachable after sys_exit!\n");
}
