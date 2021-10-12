#include "stdio.h"

int main() {
  printf("Hello, world!\n");
  asm volatile("sret");
  return 0;
}
