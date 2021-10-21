// ref: xv6-riscv by MIT (https://github.com/mit-pdos/xv6-riscv-fall19)

#include "external.h"
#include "log.h"
#include "riscv.h"
#include "stdio.h"
#include "string.h"

// Mutual exclusion spin locks.

uint64_t ntest_and_set;

void initlock(struct spinlock *lk, char *name) {
  lk->name = name;
  lk->locked = 0;
}

// Acquire the lock.
// Loops (spins) until the lock is acquired.
void acquire(struct spinlock *lk) {
  push_off(); // disable interrupts to avoid deadlock.
  if (holding(lk))
    panic("acquire");

  // On RISC-V, sync_lock_test_and_set turns into an atomic swap:
  //   a5 = 1
  //   s1 = &lk->locked
  //   amoswap.w.aq a5, a5, (s1)
  while (__sync_lock_test_and_set(&lk->locked, 1) != 0) {
    __sync_fetch_and_add(&ntest_and_set, 1);
  }

  // Tell the C compiler and the processor to not move loads or stores
  // past this point, to ensure that the critical section's memory
  // references happen after the lock is acquired.
  __sync_synchronize();
}

// Release the lock.
void release(struct spinlock *lk) {
  if (!holding(lk))
    panic("release");

  // Tell the C compiler and the CPU to not move loads or stores
  // past this point, to ensure that all the stores in the critical
  // section are visible to other CPUs before the lock is released.
  // On RISC-V, this turns into a fence instruction.
  __sync_synchronize();

  // Release the lock, equivalent to lk->locked = 0.
  // This code doesn't use a C assignment, since the C standard
  // implies that an assignment might be implemented with
  // multiple store instructions.
  // On RISC-V, sync_lock_release turns into an atomic swap:
  //   s1 = &lk->locked
  //   amoswap.w zero, zero, (s1)
  __sync_lock_release(&lk->locked);

  pop_off();
}

// Check whether this cpu is holding the lock.
int holding(struct spinlock *lk) {
  int r;
  push_off();
  r = lk->locked;
  pop_off();
  return r;
}

void push_off(void) { intr_off(); }

void pop_off(void) { intr_on(); }

uint64_t sys_ntas(void) { return ntest_and_set; }
