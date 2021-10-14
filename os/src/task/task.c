#include "task.h"

const uint64_t *get_task_cx_ptr2(TaskControlBlock *s) {
  return &(s->task_cx_ptr);
}
