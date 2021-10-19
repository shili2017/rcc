#include "task.h"
#include "trap.h"

void task_context_goto_trap_return(TaskContext *cx) {
  cx->ra = (uint64_t)trap_return;
  for (int i = 0; i < 12; i++)
    cx->s[i] = 0;
}
