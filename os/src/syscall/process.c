#include <stdint.h>

#include "loader.h"
#include "log.h"
#include "mm.h"
#include "string.h"
#include "syscall.h"
#include "task.h"
#include "timer.h"

int64_t sys_exit(int exit_code) {
  info("Application exited with code %d\n", exit_code);
  task_exit_current_and_run_next(exit_code);
  panic("Unreachable in sys_exit!\n");
  return 0;
}

int64_t sys_yield() {
  task_suspend_current_and_run_next();
  return 0;
}

int64_t sys_set_priority(int64_t prio) {
  if (prio < 2) {
    return -1;
  }
  // todo: implement set_priority
  return prio;
}

int64_t sys_get_time(TimeVal *ts, int64_t tz) {
  TimeVal sys_ts;
  int64_t time_us = timer_get_time_us();
  sys_ts.sec = time_us / USEC_PER_SEC;
  sys_ts.usec = time_us % USEC_PER_SEC;
  copy_byte_buffer(processor_current_user_token(), (uint8_t *)&sys_ts,
                   (uint8_t *)ts, sizeof(TimeVal), TO_USER);
  return 0;
}

int64_t sys_getpid() {
  TaskControlBlock *task = processor_current_task();
  return (int64_t)task->pid;
}

int64_t sys_munmap(uint64_t start, uint64_t len) {
  MemorySet *memory_set = task_current_memory_set();
  return memory_set_munmap(memory_set, start, len);
}

int64_t sys_fork() {
  TaskControlBlock *current_task = processor_current_task();
  TaskControlBlock *new_task = task_control_block_fork(current_task);
  PidHandle new_pid = new_task->pid;

  // modify trap context of new_task, because it returns immediately after
  // switching
  TrapContext *trap_cx = task_control_block_get_trap_cx(new_task);

  // we do not have to move to next instruction since we have done it before
  // for child process, fork returns 0
  trap_cx->x[10] = 0;

  // add new task to scheduler
  task_manager_add_task(new_task);

  return (int64_t)new_pid;
}

int64_t sys_exec(char *path) {
  char app_name[MAX_APP_NAME_LENGTH];
  copy_byte_buffer(processor_current_user_token(), (uint8_t *)app_name,
                   (uint8_t *)path, MAX_APP_NAME_LENGTH, FROM_USER);

  uint8_t *data = loader_get_app_data_by_name(app_name);
  size_t size = loader_get_app_size_by_name(app_name);
  TaskControlBlock *task;

  if (data) {
    task = processor_current_task();
    task_control_block_exec(task, data, size);
    return 0;
  } else {
    return -1;
  }
}

int64_t sys_mmap(uint64_t start, uint64_t len, uint64_t prot) {
  MemorySet *memory_set = task_current_memory_set();
  return memory_set_mmap(memory_set, start, len, prot);
}

int64_t sys_waitpid(int64_t pid, int *exit_code_ptr) {
  TaskControlBlock *task = processor_current_task();

  // find a child process
  bool found = false;
  uint64_t found_idx;
  PidHandle found_pid;
  int exit_code;
  TaskControlBlock *x = (TaskControlBlock *)(task->children.buffer);
  for (uint64_t i = 0; i < task->children.size; i++) {
    if (x[i].pid == task_control_block_getpid(&x[i]) || x[i].pid == -1) {
      found = true;
      found_idx = i;
      found_pid = x[i].pid;
      exit_code = x[i].exit_code;
      break;
    }
  }
  if (!found) {
    return -1;
  }

  if (x[found_idx].task_status == TASK_STATUS_ZOMBIE) {
    vector_remove(&task->children, found_idx);
    copy_byte_buffer(memory_set_token(&task->memory_set), (uint8_t *)&exit_code,
                     (uint8_t *)exit_code_ptr, sizeof(int), TO_USER);
    return (int64_t)found_pid;
  } else {
    return -2;
  }
}
