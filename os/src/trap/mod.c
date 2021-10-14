#include "log.h"
#include "syscall.h"
#include "task.h"
#include "trap.h"

extern void __alltraps();

void trap_init() {
  // initialize trap entry (direct mode)
  asm volatile("csrw stvec, %0" ::"r"(__alltraps));
}

void trap_enable_timer_interrupt() {
  uint64_t sie;
  uint64_t sie_stie = 0x20;
  asm volatile("csrr %0, sie" : "=r"(sie));
  sie = sie | sie_stie;
  asm volatile("csrw sie, %0" ::"r"(sie));
}

TrapContext *trap_handler(TrapContext *c) {
  uint64_t scause;
  uint64_t stval;
  asm volatile("csrr %0, scause\n"
               "csrr %1, stval\n"
               : "=r"(scause), "=r"(stval));

  uint64_t scause_cause = scause & ~(1L << 63);
  switch (scause_cause) {
  case UserEnvCall:
    c->sepc += 4;
    c->x[10] = syscall(c->x[17], c->x[10], c->x[11], c->x[12]);
    break;
  case StoreFault:
  case StorePageFault:
    info("PageFault in application, bad addr = %llx, bad instruction = %llx, "
         "core dumped.\n",
         stval, c->sepc);
    task_exit_current_and_run_next();
    break;
  case IllegalInstruction:
    info("IllegalInstruction in application, core dumped.\n");
    task_exit_current_and_run_next();
    break;
  default:
    panic("Unsupported trap 0x%llx, stval = 0x%llx\n", scause_cause, stval);
    break;
  }

  return c;
}
