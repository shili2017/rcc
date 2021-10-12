#include <stdint.h>

#include "batch.h"
#include "log.h"
#include "printf.h"
#include "sbi.h"
#include "trap.h"

// putchar function to console
inline void _putchar(char character) { console_putchar(character); }

extern uint8_t stext, etext;
extern uint8_t srodata, erodata;
extern uint8_t sdata, edata;
extern uint8_t sbss, ebss;
extern uint8_t boot_stack, boot_stack_top;

void clear_bss() {
  for (uint8_t *i = &sbss; i < &ebss; i++) {
    *i = 0;
  }
}

int main() {
  clear_bss();
  info("Hello, world!\n");

  trap_init();
  batch_init();
  batch_run_next_app();

  shutdown();
  return 0;
}
