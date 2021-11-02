#ifndef _FS_H_
#define _FS_H_

#include <stdbool.h>
#include <stdint.h>

#define PIPE_SIZE 256

typedef struct {
  char buffer[PIPE_SIZE];
  uint64_t read_bytes;  // number of bytes read
  uint64_t write_bytes; // number of bytes written
  bool read_open;
  bool write_open;
} Pipe;

typedef struct {
  int64_t ref;
  Pipe *pipe;
  bool is_pipe;
  bool readable;
  bool writable;
} File;

// fs.c
int64_t stdin_read(char *buf, uint64_t len);
int64_t stdout_write(char *buf, uint64_t len);

// pipe.c
int64_t pipe_make(File *f0, File *f1);
int64_t pipe_close(Pipe *pipe, bool writable);
int64_t pipe_read(Pipe *pipe, char *buf, uint64_t len);
int64_t pipe_write(Pipe *pipe, char *buf, uint64_t len);

#endif // _FS_H_
