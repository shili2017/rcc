#include <stdint.h>

#include "stdio.h"
#include "syscall.h"

int main() {
  printf("into sleep test!\n");
  int64_t start = get_time();
  printf("current time_msec = %lld\n", start);
  sleep(100);
  int64_t end = get_time();
  printf("time_msec = %lld after sleeping 100 ticks, delta = %lld ms!\n", end,
         end - start);
  printf("r_sleep passed!\n");
  return 0;
}
