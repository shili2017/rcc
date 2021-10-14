#include "task.h"

extern void __restore(uint64_t);

TaskContext *task_context_goto_restore(TaskContext *c) {
  c->ra = (uint64_t)__restore;
  for (unsigned i = 0; i < 12; i++)
    c->s[i] = 0;
  return c;
}
