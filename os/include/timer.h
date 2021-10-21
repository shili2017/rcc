#ifndef _TIMER_H_
#define _TIMER_H_

#define TICKS_PER_SEC 100
#define USEC_PER_SEC 1000000

typedef struct {
  uint64_t sec;
  uint64_t usec;
} TimeVal;

uint64_t timer_get_time();
uint64_t timer_get_time_us();
void timer_set_next_trigger();

#endif // _TIMER_H_
