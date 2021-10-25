#include <stdint.h>

#include "loader.h"
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

  mm_remap_test();

  task_add_initproc();

  trap_init();

  trap_enable_timer_interrupt();

  timer_set_next_trigger();

  loader_init_and_list_apps();

  processor_run_tasks();

  panic("Unreachable in main!\n");
}
