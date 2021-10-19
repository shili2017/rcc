#include "task.h"

void task_init() {
  // Task init
  task_manager_init();
}

void task_run_first_task() {
  // Task run_first_task
  task_manager_run_first_task();
}

void task_run_next_task() {
  // Task run_next_task
  task_manager_run_next_task();
}

void task_mark_current_suspended() {
  // Task mark_current_suspended
  task_manager_mark_current_suspended();
}

void task_mark_current_exited() {
  // Task mark_current_exited
  task_manager_mark_current_exited();
}

void task_suspend_current_and_run_next() {
  // Task suspend_current_and_run_next
  task_mark_current_suspended();
  task_run_next_task();
}

void task_exit_current_and_run_next() {
  // Task exit_current_and_run_next
  task_mark_current_exited();
  task_run_next_task();
}

uint64_t task_get_current_task() {
  // Task get_current_task
  return task_manager_get_current_task();
}

void task_set_priority(int64_t prio) {
  // Task set_priority
  task_manager_set_priority(prio);
}

uint64_t task_current_user_token() {
  // Task current_token
  return task_manager_get_current_token();
}

TrapContext *task_current_trap_cx() {
  // Task current_trap_cx
  return task_manager_get_current_trap_cx();
}
