#include <stdint.h>

#include "drivers.h"
#include "fs.h"
#include "log.h"
#include "task.h"
#include "timer.h"
#include "trap.h"

extern uint8_t sbss, ebss;

void clear_bss() {
  for (uint8_t *i = &sbss; i < &ebss; i++) {
    *i = 0;
  }
}

void main() {
  clear_bss();

  info("Hello, world!\n");

  mm_init();

  trap_init();

  plic_init();

  inode_root_init();

  task_init();

  trap_enable_timer_interrupt();

  timer_set_next_trigger();

  processor_run_tasks();

  panic("Unreachable in main!\n");
}
