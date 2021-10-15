#include "trap.h"
#include "log.h"
#include "syscall.h"
#include "task.h"
#include "timer.h"

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
