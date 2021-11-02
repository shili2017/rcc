#include "external.h"
#include "fs.h"
#include "log.h"
#include "mm.h"
#include "string.h"
#include "task.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))

int64_t pipe_make(File *f0, File *f1) {
  Pipe *pipe = bd_malloc(sizeof(Pipe));
  memset(pipe, 0, sizeof(Pipe));
  pipe->read_bytes = 0;
  pipe->write_bytes = 0;
  pipe->read_open = true;
  pipe->write_open = true;

  f0->ref = 1;
  f0->pipe = pipe;
  f0->is_pipe = true;
  f0->readable = true;
  f0->writable = false;

  f1->ref = 1;
  f1->pipe = pipe;
  f1->is_pipe = true;
  f1->readable = false;
  f1->writable = true;

  return 0;
}

int64_t pipe_close(Pipe *pipe, bool writable) {
  if (writable) {
    pipe->write_open = false;
  } else {
    pipe->read_open = false;
  }
  if (!pipe->read_open && !pipe->write_open) {
    bd_free(pipe);
  }
  return 0;
}

int64_t pipe_read(Pipe *pipe, char *buf, uint64_t len) {
  assert(len > 1, "pipe_read len <= 0\n");

  uint64_t i = 0;
  uint64_t size = -1;

  while (pipe->read_bytes == pipe->write_bytes) {
    if (pipe->write_open) {
      task_suspend_current_and_run_next();
    } else {
      return -1;
    }
  }

  while (i < len && size != 0) {
    if (pipe->read_bytes == pipe->write_bytes)
      break;
    size = MIN(MIN(len - i, pipe->write_bytes - pipe->read_bytes),
               PIPE_SIZE - (pipe->read_bytes % PIPE_SIZE));
    copy_byte_buffer(processor_current_user_token(),
                     (uint8_t *)&pipe->buffer[pipe->read_bytes % PIPE_SIZE],
                     (uint8_t *)buf + i, size, TO_USER);
    pipe->read_bytes += size;
    i += size;
  }

  return i;
}

int64_t pipe_write(Pipe *pipe, char *buf, uint64_t len) {
  assert(len > 1, "pipe_write len <= 0\n");

  uint64_t i = 0;
  uint64_t size = -1;

  while (i < len) {
    if (!pipe->read_open) {
      return -1;
    }
    if (pipe->write_bytes == pipe->read_bytes + PIPE_SIZE) {
      task_suspend_current_and_run_next();
    } else {
      size = MIN(MIN(len - i, pipe->read_bytes + PIPE_SIZE - pipe->write_bytes),
                 PIPE_SIZE - (pipe->write_bytes % PIPE_SIZE));
      copy_byte_buffer(processor_current_user_token(),
                       (uint8_t *)&pipe->buffer[pipe->write_bytes % PIPE_SIZE],
                       (uint8_t *)buf + i, size, TO_USER);
      pipe->write_bytes += size;
      i += size;
    }
  }

  return i;
}
