#ifndef _TRAP_H_
#define _TRAP_H_

#include <stdint.h>

typedef struct TrapContext TrapContext;

struct TrapContext {
  uint64_t x[32];
  uint64_t sstatus;
  uint64_t sepc;
};

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
TrapContext *app_init_context(uint64_t entry, uint64_t sp, TrapContext *c);

#endif // _TRAP_H_
