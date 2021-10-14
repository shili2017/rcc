#include <stdint.h>

#include "config.h"
#include "sbi.h"
#include "timer.h"

#define TICKS_PER_SEC 100
#define MSEC_PER_SEC 1000

uint64_t timer_get_time() {
  uint64_t time;
  asm volatile("rdtime %0" : "=r"(time));
  return time;
}

uint64_t timer_get_time_ms() {
  return timer_get_time() / (CLOCK_FREQ / MSEC_PER_SEC);
}

void timer_set_next_trigger() {
  set_timer(timer_get_time() + CLOCK_FREQ / TICKS_PER_SEC);
}
