#include "sbi.h"
#include "printf.h"
#include "log.h"

// putchar function to console
inline void _putchar(char character) {
  console_putchar(character);
}

extern char stext, etext;
extern char srodata, erodata;
extern char sdata, edata;
extern char sbss, ebss;
extern char boot_stack, boot_stack_top;

void clear_bss() {
  for (char *i = &sbss; i < &ebss; i++) {
    *i = 0;
    info("%llx\n", i);
  }
}

int main() {
  clear_bss();
  printf("Hello, world!\n");
  info(".text      [0x%llx, 0x%llx)\n", &stext     , &etext         );
  info(".rodata    [0x%llx, 0x%llx)\n", &srodata   , &erodata       );
  info(".data      [0x%llx, 0x%llx)\n", &sdata     , &edata         );
  info("boot_stack [0x%llx, 0x%llx)\n", &boot_stack, &boot_stack_top);
  info(".bss       [0x%llx, 0x%llx)\n", &sbss      , &ebss          );
  shutdown();
  return 0;
}
