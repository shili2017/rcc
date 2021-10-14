#include <stdint.h>

#include "log.h"
#include "stdio.h"
#include "task.h"
#include "timer.h"

int64_t sys_exit(int exit_code) {
  info("Application exited with code %d\n", exit_code);
  task_exit_current_and_run_next();
  panic("Unreachable in sys_exit!\n");
  return 0;
}

int64_t sys_yield() {
  task_suspend_current_and_run_next();
  return 0;
}

int64_t sys_get_time() {
  int64_t time_ms = timer_get_time_ms();
  return time_ms;
}
