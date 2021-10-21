#include <stdint.h>

#include "loader.h"
#include "log.h"
#include "sbi.h"
#include "stdio.h"
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

  info("Back to world!\n");

  mm_remap_test();

  trap_init();

  trap_enable_timer_interrupt();

  timer_set_next_trigger();

  task_init();

  task_run_first_task();

  panic("Unreachable in main!\n");
}
