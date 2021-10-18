#include "riscv.h"
#include "trap.h"

TrapContext *app_init_context(uint64_t entry, uint64_t sp, TrapContext *c) {
  uint64_t sstatus = r_sstatus();
  sstatus &= ~SSTATUS_SPP;

  for (int i = 0; i < 32; i++)
    c->x[i] = 0;
  c->sstatus = sstatus;
  c->sepc = entry;
  c->x[2] = sp;

  return c;
}
