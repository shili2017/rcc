#include "trap.h"

TrapContext *app_init_context(uint64_t entry, uint64_t sp, TrapContext *c) {
  uint64_t sstatus;
  asm volatile("csrr %0, sstatus" : "=r"(sstatus));
  sstatus &= ~(1L << 8);

  for (int i = 0; i < 32; i++)
    c->x[i] = 0;
  c->sstatus = sstatus;
  c->sepc = entry;
  c->x[2] = sp;

  return c;
}
