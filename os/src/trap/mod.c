#include "batch.h"
#include "log.h"
#include "trap.h"

extern void __alltraps();

void trap_init() {
  // initialize trap entry
  asm volatile("csrw stvec, %0" ::"r"(__alltraps));
}

TrapContext *trap_handler(TrapContext *c) {
  uint64_t scause;
  uint64_t stval;
  asm volatile("csrr %0, scause\n"
               "csrr %1, stval\n"
               : "=r"(scause), "=r"(stval));

  switch (scause) {
  case UserEnvCall:
    c->sepc += 4;
    c->x[10] = syscall(c->x[17], c->x[10], c->x[11], c->x[12]);
  case StoreFault:
  case StorePageFault:
    info("PageFault in application, core dumped.\n");
    batch_run_next_app();
    break;
  case IllegalInstruction:
    info("IllegalInstruction in application, core dumped.\n");
    batch_run_next_app();
    break;
  default:
    panic("Unsupported trap 0x%llx, stval = 0x%llx\n", scause, stval);
    break;
  }

  return c;
}
