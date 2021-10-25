#include "task.h"

extern void __switch(TaskContext *current_task_cx_ptr,
                     const TaskContext *next_task_cx_ptr);

static Processor PROCESSOR;

static inline void processor_init(Processor *processor) {
  processor->current = NULL;
  task_context_zero_init(&processor->idle_task_cx);
}

static TaskContext *processor_get_idle_task_cx_ptr(Processor *processor) {
  return &processor->idle_task_cx;
}

static TaskControlBlock *processor_take_current(Processor *processor) {
  return processor->current;
}

static TaskControlBlock *processor_current(Processor *processor) {
  return processor->current;
}

void processor_run_tasks() {
  processor_init(&PROCESSOR);
  TaskControlBlock *task;
  TaskContext *idle_task_cx_ptr;
  TaskContext *next_task_cx_ptr;
  while (1) {
    task = task_manager_fetch_task();
    if (task) {
      idle_task_cx_ptr = processor_get_idle_task_cx_ptr(&PROCESSOR);
      next_task_cx_ptr = &task->task_cx;
      task->task_status = TASK_STATUS_RUNNING;
      PROCESSOR.current = task;
      __switch(idle_task_cx_ptr, next_task_cx_ptr);
    }
  }
}

TaskControlBlock *processor_take_current_task() {
  return processor_take_current(&PROCESSOR);
}

TaskControlBlock *processor_current_task() {
  return processor_current(&PROCESSOR);
}

uint64_t processor_current_user_token() {
  TaskControlBlock *task = processor_current_task();
  return task_control_block_get_user_token(task);
}

TrapContext *processor_current_trap_cx() {
  TaskControlBlock *task = processor_current_task();
  return task_control_block_get_trap_cx(task);
}

void processor_schedule(TaskContext *switched_task_cx_ptr) {
  TaskContext *idle_task_cx_ptr = processor_get_idle_task_cx_ptr(&PROCESSOR);
  __switch(switched_task_cx_ptr, idle_task_cx_ptr);
}
