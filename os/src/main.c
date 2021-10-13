#include <stdint.h>

#include "batch.h"
#include "log.h"
#include "sbi.h"
#include "stdio.h"
#include "trap.h"

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
  debug(".text      [0x%llx, 0x%llx)\n", &stext, &etext);
  debug(".rodata    [0x%llx, 0x%llx)\n", &srodata, &erodata);
  debug(".data      [0x%llx, 0x%llx)\n", &sdata, &edata);
  debug("boot_stack [0x%llx, 0x%llx)\n", &boot_stack, &boot_stack_top);
  debug(".bss       [0x%llx, 0x%llx)\n", &sbss, &ebss);

  trap_init();
  batch_init();
  batch_run_next_app();

  shutdown();
  return 0;
}
