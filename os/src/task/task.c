#include "task.h"
#include "loader.h"

static TaskControlBlock INITPROC;

void task_init() {
  pid_allocator_init();
  task_manager_init();
  task_control_block_new(&INITPROC, loader_get_app_data_by_name("initproc"),
                         loader_get_app_size_by_name("initproc"));
}

void task_add_initproc() { task_manager_add_task(&INITPROC); }

void task_suspend_current_and_run_next() {
  // There must be an application running.
  TaskControlBlock *task = processor_take_current_task();
  TaskContext *task_cx_ptr = &task->task_cx;

  // Change status to Ready
  task->task_status = TASK_STATUS_READY;

  // push back to ready queue
  task_manager_add_task(task);

  // jump to scheduling cycle
  processor_schedule(task_cx_ptr);
}

void task_exit_current_and_run_next(int exit_code) {
  // take from Processor
  TaskControlBlock *task = processor_take_current_task();

  // Change status to Zombie
  task->task_status = TASK_STATUS_ZOMBIE;

  // Record exit code
  task->exit_code = exit_code;
  // do not move to its parent but under initproc

  TaskControlBlock **x = (TaskControlBlock **)(task->children.buffer);
  for (uint64_t i = 0; i < task->children.size; i++) {
    x[i]->parent = &INITPROC;
    vector_push(&INITPROC.children, x[i]);
  }
  vector_free(&task->children);

  // deallocate user space
  memory_set_recycle_data_pages(&task->memory_set);

  // deallocate kernel stack
  kernel_stack_free(&task->kernel_stack);

  // we do not have to save task context
  TaskContext _unused;
  task_context_zero_init(&_unused);
  processor_schedule(&_unused);
}

MemorySet *task_current_memory_set() {
  TaskControlBlock *task = processor_take_current_task();
  return &task->memory_set;
}
