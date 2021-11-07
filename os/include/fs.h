#ifndef _FS_H_
#define _FS_H_

#include <stdbool.h>
#include <stdint.h>

#include "efs.h"

#define PIPE_SIZE 256
#define MAX_MAIL_NUM 16
#define MAIL_SIZE 256

typedef struct {
  char buffer[PIPE_SIZE];
  uint64_t read_bytes;  // number of bytes read
  uint64_t write_bytes; // number of bytes written
  bool read_open;
  bool write_open;
} Pipe;

typedef struct {
  Inode inode;
  int64_t ref;
  uint64_t offset;
} OSInode;

typedef struct {
  int64_t proc_ref;
  int64_t file_ref;
  Pipe *pipe;     // FD_PIPE
  OSInode *inode; // FD_INODE
  enum { FD_NONE = 0, FD_PIPE, FD_INODE } type;
  bool readable;
  bool writable;
} File;

typedef struct {
  char buffer[MAX_MAIL_NUM][MAIL_SIZE];
  uint64_t read_mails;  // number of mails read
  uint64_t write_mails; // number of mails written
} Mailbox;

// fs.c
int64_t stdin_read(char *buf, uint64_t len);
int64_t stdout_write(char *buf, uint64_t len);

// pipe.c
int64_t pipe_make(File *f0, File *f1);
int64_t pipe_close(Pipe *pipe, bool writable);
int64_t pipe_read(Pipe *pipe, char *buf, uint64_t len);
int64_t pipe_write(Pipe *pipe, char *buf, uint64_t len);

// inode.c
uint64_t inode_read_all(OSInode *osinode, uint8_t *buf);
void inode_root_init();
void inode_list_apps();
OSInode *inode_open_file(char *name, uint32_t flags);
int64_t inode_close_file(OSInode *osinode);
int64_t inode_read(OSInode *osinode, char *buf, uint64_t len);
int64_t inode_write(OSInode *osinode, char *buf, uint64_t len);

#endif // _FS_H_
