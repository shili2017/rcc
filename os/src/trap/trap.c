#include "trap.h"
#include "log.h"
#include "riscv.h"
#include "syscall.h"
#include "task.h"
#include "timer.h"

void trap_from_kernel() {
  uint64_t scause = r_scause();
  uint64_t sepc = r_sepc();
  uint64_t stval = r_stval();

  panic("A trap from kernel! (scause = 0x%llx, sepc = 0x%llx stval = 0x%llx)\n",
        scause, sepc, stval);
}

static inline void set_kernel_trap_entry() {
  // write to stvec - trap_from_kernel
  w_stvec((uint64_t)trap_from_kernel);
}

static inline void set_user_trap_entry() {
  // write to stvec - TRAMPOLINE
  w_stvec((uint64_t)TRAMPOLINE);
}

void trap_init() {
  // Trap init
  set_kernel_trap_entry();
}

void trap_enable_timer_interrupt() {
  uint64_t sie = r_sie();
  sie = sie | SIE_STIE;
  w_sie(sie);
}

void trap_handler() {
  set_kernel_trap_entry();

  TrapContext *cx = processor_current_trap_cx();
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
      cx->sepc += 4;
      cx->x[10] = syscall(cx->x[17], cx->x[10], cx->x[11], cx->x[12]);
      break;
    case StoreFault:
    case StorePageFault:
    case InstructionFault:
    case InstructionPageFault:
    case LoadFault:
    case LoadPageFault:
      info("Exception %lld in application, bad addr = %llx, bad instruction = "
           "%llx, core dumped.\n",
           scause, stval, cx->sepc);
      // page fault exit code
      task_exit_current_and_run_next(-2);
      break;
    case IllegalInstruction:
      info("IllegalInstruction in application, core dumped.\n");
      // illegal instruction exit code
      task_exit_current_and_run_next(-3);
      break;
    default:
      panic("Unsupported exception 0x%llx, stval = 0x%llx\n", scause, stval);
      break;
    }
  }

  trap_return();
}

extern void __alltraps();
extern void __restore();

void trap_return() {
  set_user_trap_entry();
  uint64_t trap_cx_ptr = TRAP_CONTEXT;
  uint64_t user_satp = processor_current_user_token();
  uint64_t restore_va = (uint64_t)__restore - (uint64_t)__alltraps + TRAMPOLINE;
  asm volatile("fence.i");
  asm volatile("mv x10, %1\n"
               "mv x11, %2\n"
               "jr %0\n"
               :
               : "r"(restore_va), "r"(trap_cx_ptr), "r"(user_satp)
               : "memory", "x10", "x11");
  panic("Unreachable in back_to_user!\n");
}
