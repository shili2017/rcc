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

int64_t sys_set_priority(int64_t prio) {
  if (prio < 2) {
    return -1;
  }
  task_set_priority(prio);
  return prio;
}

int64_t sys_get_time(TimeVal *ts, int64_t tz) {
  int64_t time_us = timer_get_time_us();
  ts->sec = time_us / USEC_PER_SEC;
  ts->usec = time_us % USEC_PER_SEC;
  return 0;
}
