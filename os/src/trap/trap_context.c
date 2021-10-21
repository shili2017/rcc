#include "riscv.h"
#include "trap.h"

void app_init_context(uint64_t entry, uint64_t sp, uint64_t kernel_satp,
                      uint64_t kernel_sp, uint64_t trap_handler,
                      TrapContext *c) {
  uint64_t sstatus = r_sstatus();
  sstatus &= ~SSTATUS_SPP;

  for (int i = 0; i < 32; i++)
    c->x[i] = 0;
  c->sstatus = sstatus;
  c->sepc = entry;
  c->kernel_satp = kernel_satp;
  c->kernel_sp = kernel_sp;
  c->trap_handler = trap_handler;
  c->x[2] = sp;
}
