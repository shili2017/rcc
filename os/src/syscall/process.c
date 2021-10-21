#include <stdint.h>

#include "log.h"
#include "mm.h"
#include "stdio.h"
#include "syscall.h"
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
  TimeVal sys_ts;
  int64_t time_us = timer_get_time_us();
  sys_ts.sec = time_us / USEC_PER_SEC;
  sys_ts.usec = time_us % USEC_PER_SEC;
  copy_byte_buffer(task_current_user_token(), (uint8_t *)&sys_ts, (uint8_t *)ts,
                   sizeof(TimeVal), TO_USER);
  return 0;
}

int64_t sys_mmap(uint64_t start, uint64_t len, uint64_t prot) {
  MemorySet *memory_set = task_current_memory_set();
  return memory_set_mmap(memory_set, start, len, prot);
}

int64_t sys_munmap(uint64_t start, uint64_t len) {
  MemorySet *memory_set = task_current_memory_set();
  return memory_set_munmap(memory_set, start, len);
}
