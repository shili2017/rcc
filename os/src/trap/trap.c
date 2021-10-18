#include "trap.h"
#include "log.h"
#include "riscv.h"
#include "syscall.h"
#include "task.h"
#include "timer.h"

extern void __alltraps();

void trap_init() {
  // initialize trap entry (direct mode)
  w_stvec((uint64_t)__alltraps);
}

void trap_enable_timer_interrupt() {
  uint64_t sie = r_sie();
  sie = sie | SIE_STIE;
  w_sie(sie);
}

TrapContext *trap_handler(TrapContext *c) {
  uint64_t scause = r_scause();
  uint64_t stval = r_stval();

  if (scause & (1ULL << 63)) {
    scause &= ~(1ULL << 63);
    switch (scause) {
    case SupervisorTimer:
      timer_set_next_trigger();
      task_suspend_current_and_run_next();
      break;
    default:
      panic("Unsupported interrupt 0x%llx, stval = 0x%llx\n", scause, stval);
      break;
    }
  } else {
    switch (scause) {
    case UserEnvCall:
      c->sepc += 4;
      c->x[10] = syscall(c->x[17], c->x[10], c->x[11], c->x[12]);
      break;
    case StoreFault:
    case StorePageFault:
    case InstructionFault:
    case InstructionPageFault:
    case LoadFault:
    case LoadPageFault:
      info("Exception %lld in application, bad addr = %llx, bad instruction = "
           "%llx, core dumped.\n",
           scause, stval, c->sepc);
      task_exit_current_and_run_next();
      break;
    case IllegalInstruction:
      info("IllegalInstruction in application, core dumped.\n");
      task_exit_current_and_run_next();
      break;
    default:
      panic("Unsupported exception 0x%llx, stval = 0x%llx\n", scause, stval);
      break;
    }
  }

  return c;
}
