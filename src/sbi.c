#include "sbi.h"
#include "log.h"

static inline int64_t sbi_call(uint64_t id, uint64_t a0, uint64_t a1, uint64_t a2) {
  int64_t ret;
  asm volatile (
    "mv x10, %1\n"
    "mv x11, %2\n"
    "mv x12, %3\n"
    "mv x17, %4\n"
    "ecall\n"
    "mv %0, x10\n"
    :"=r"(ret)
    :"r"(a0), "r"(a1), "r"(a2), "r"(id)
    :"memory", "x10", "x11", "x12", "x17"
  );
  return ret;
}

void console_putchar(char c) {
  sbi_call(SBI_CONSOLE_PUTCHAR, c, 0, 0);
}

void console_getchar() {
  sbi_call(SBI_CONSOLE_GETCHAR, 0, 0, 0);
}

void shutdown() {
  sbi_call(SBI_SHUTDOWN, 0, 0, 0);
  panic("Shutdown now...");
}
