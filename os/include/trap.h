#ifndef _TRAP_H_
#define _TRAP_H_

#include <stdint.h>

typedef struct {
  uint64_t x[32];
  uint64_t sstatus;
  uint64_t sepc;
  uint64_t kernel_satp;
  uint64_t kernel_sp;
  uint64_t trap_handler;
} TrapContext;

// Interrupt

#define UserSoft 0
#define SupervisorSoft 1
#define UserTimer 4
#define SupervisorTimer 5
#define UserExternal 8
#define SupervisorExternal 9

// Exception

#define InstructionMisaligned 0
#define InstructionFault 1
#define IllegalInstruction 2
#define Breakpoint 3
#define LoadMisaligned 4
#define LoadFault 5
#define StoreMisaligned 6
#define StoreFault 7
#define UserEnvCall 8
#define SupervisorEnvCall 9
#define MachineEnvCall 11
#define InstructionPageFault 12
#define LoadPageFault 13
#define StorePageFault 15

void trap_init();
void trap_from_kernel();
void trap_enable_timer_interrupt();
void trap_handler();
void trap_return();

void app_init_context(uint64_t entry, uint64_t sp, uint64_t kernel_satp,
                      uint64_t kernel_sp, uint64_t trap_handler,
                      TrapContext *c);

#endif // _TRAP_H_
