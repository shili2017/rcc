#include <stdint.h>

#include "stdio.h"
#include "syscall.h"

int main() {
  int64_t current_timer = get_time();
  int64_t wait_for = current_timer + 3000;

  while (get_time() < wait_for) {
    current_timer = get_time();
    wait_for = current_timer + 3000;
    yield();
  }

  printf("Test sleep OK!\n");
  return 0;
}
