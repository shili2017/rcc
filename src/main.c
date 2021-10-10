#include "sbi.h"
#include "printf.h"

extern char stext, etext, srodata, erodata, sdata, edata, sbss, ebss;
extern char boot_stack, boot_stack_top;

void clear_bss() {
  for (char *i = &sbss; i < &ebss; i++)
    *i = 0;
}

int main() {
  clear_bss();
  printf("Hello, world!\n");
  shutdown();
  return 0;
}
