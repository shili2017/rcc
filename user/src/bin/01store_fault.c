#include <stdint.h>

#include "stdio.h"

int main() {
  printf("Test store_fault, we will insert an invalid store operation...\n");
  printf("Kernel should kill this application!\n");
  uint64_t *addr = NULL;
  *addr = 0;
  return 0;
}
