#include "task.h"

TaskManager TASK_MANAGER;

void task_manager_new() {
  queue_new(&TASK_MANAGER.ready_queue, sizeof(TaskControlBlock));
}

void task_manager_free() { queue_free(&TASK_MANAGER.ready_queue); }

void task_manager_add(TaskManager *tm, TaskControlBlock *task) {
  queue_push(&tm->ready_queue, task);
}

TaskControlBlock *task_manager_fetch(TaskManager *tm) {
  if (queue_empty(&tm->ready_queue)) {
    return NULL;
  }
  TaskControlBlock *task = (TaskControlBlock *)queue_front(&tm->ready_queue);
  queue_pop(&tm->ready_queue);
  return task;
}

void task_manager_add_task(TaskControlBlock *task) {
  task_manager_add(&TASK_MANAGER, task);
}

TaskControlBlock *task_manager_fetch_task() {
  return task_manager_fetch(&TASK_MANAGER);
}
