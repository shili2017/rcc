#include <stdint.h>

#include "fs.h"
#include "loader.h"
#include "log.h"
#include "mm.h"
#include "sbi.h"
#include "stdio.h"
#include "string.h"
#include "syscall.h"
#include "task.h"
#include "timer.h"

int64_t sys_close(uint64_t fd) {
  TaskControlBlock *task = processor_current_task();
  File *file = task->fd_table[fd];

  if (fd <= 2 || fd > MAX_FILE_NUM || !file || file->ref < 1) {
    return -1;
  }
  if (--file->ref > 0) {
    return 0;
  }
  if (file->is_pipe) {
    pipe_close(file->pipe, file->writable);
  }

  bd_free(file);
  file = NULL;
  return 0;
}

int64_t sys_pipe(uint64_t *pipe) {
  TaskControlBlock *task = processor_current_task();
  uint64_t token = processor_current_user_token();

  int64_t read_fd = task_control_block_alloc_fd(task);
  int64_t write_fd = task_control_block_alloc_fd(task);

  if (read_fd < 0 || write_fd < 0) {
    return -1;
  }

  File *pipe_read = task->fd_table[read_fd];
  File *pipe_write = task->fd_table[write_fd];
  pipe_make(pipe_read, pipe_write);

  copy_byte_buffer(token, (uint8_t *)&read_fd, (uint8_t *)(&pipe[0]),
                   sizeof(uint64_t), TO_USER);
  copy_byte_buffer(token, (uint8_t *)&write_fd, (uint8_t *)(&pipe[1]),
                   sizeof(uint64_t), TO_USER);
  return 0;
}

int64_t sys_read(uint64_t fd, char *buf, uint64_t len) {
  TaskControlBlock *task = processor_current_task();
  File *file = task->fd_table[fd];

  if (fd == FD_STDIN) {
    return stdin_read(buf, len);
  }
  if (fd <= 2 || fd > MAX_FILE_NUM || !file) {
    return -1;
  }
  if (file->is_pipe) {
    return pipe_read(file->pipe, buf, len);
  }

  return -1;
}

int64_t sys_write(uint64_t fd, char *buf, uint64_t len) {
  TaskControlBlock *task = processor_current_task();
  File *file = task->fd_table[fd];

  if (fd == FD_STDOUT || fd == FD_STDERR) {
    return stdout_write(buf, len);
  }
  if (fd <= 2 || fd > MAX_FILE_NUM || !file) {
    return -1;
  }
  if (file->is_pipe) {
    return pipe_write(file->pipe, buf, len);
  }
  return -1;
}

int64_t sys_exit(int exit_code) {
  info("Application exited with code %d\n", exit_code);
  debug("Remaining physical pages %lld\n", frame_remaining_pages());
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
  if (task_manager_almost_full()) {
    return -1;
  }

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
  TaskControlBlock **x = (TaskControlBlock **)(task->children.buffer);
  for (int64_t i = task->children.size - 1; i >= 0; i--) {
    if (pid == x[i]->pid || pid == -1) {
      found = true;
      found_idx = i;
      found_pid = x[i]->pid;
      exit_code = x[i]->exit_code;
      break;
    }
  }
  if (!found) {
    return -1;
  }

  TaskControlBlock *task_child = x[found_idx];

  if (task_child->task_status == TASK_STATUS_ZOMBIE) {
    task_control_block_free(task_child);
    vector_remove(&task->children, found_idx);
    copy_byte_buffer(memory_set_token(&task->memory_set), (uint8_t *)&exit_code,
                     (uint8_t *)exit_code_ptr, sizeof(int), TO_USER);
    return (int64_t)found_pid;
  } else {
    return -2;
  }
}

int64_t sys_spawn(char *path) {
  if (task_manager_almost_full()) {
    return -1;
  }

  TaskControlBlock *current_task = processor_current_task();

  char app_name[MAX_APP_NAME_LENGTH];
  copy_byte_buffer(processor_current_user_token(), (uint8_t *)app_name,
                   (uint8_t *)path, MAX_APP_NAME_LENGTH, FROM_USER);

  uint8_t *data = loader_get_app_data_by_name(app_name);
  size_t size = loader_get_app_size_by_name(app_name);
  TaskControlBlock *new_task;

  if (data) {
    new_task = task_control_block_spawn(current_task, data, size);
    task_manager_add_task(new_task);
    return (int64_t)new_task->pid;
  } else {
    return -1;
  }
}

int64_t sys_mailread(char *buf, uint64_t len) {
  TaskControlBlock *task = processor_current_task();

  if (task->mailbox.write_mails == task->mailbox.read_mails) {
    return -1;
  }
  if (len == 0) {
    return 0;
  }
  if (len > MAIL_SIZE) {
    len = MAIL_SIZE;
  }

  int64_t ret = copy_byte_buffer(
      processor_current_user_token(),
      (uint8_t *)task->mailbox.buffer[task->mailbox.read_mails % MAX_MAIL_NUM],
      (uint8_t *)buf, len, TO_USER);
  if (ret < 0) {
    return -1;
  }
  info("read mail from %lld, len = %lld\n", task->mailbox.read_mails, len);
  task->mailbox.read_mails++;
  return len;
}

int64_t sys_mailwrite(int64_t pid, char *buf, uint64_t len) {
  TaskControlBlock *task = task_manager_fetch_task_by_pid((uint64_t)pid);

  if (task->mailbox.write_mails - task->mailbox.read_mails == MAX_MAIL_NUM) {
    return -1;
  }
  if (len == 0) {
    return 0;
  }
  if (len > MAIL_SIZE) {
    len = MAIL_SIZE;
  }

  int64_t ret = copy_byte_buffer(
      processor_current_user_token(),
      (uint8_t *)task->mailbox.buffer[task->mailbox.write_mails % MAX_MAIL_NUM],
      (uint8_t *)buf, len, FROM_USER);
  if (ret < 0) {
    return -1;
  }
  info("write mail to %lld, len = %lld\n", task->mailbox.write_mails, len);
  task->mailbox.write_mails++;
  return len;
}
