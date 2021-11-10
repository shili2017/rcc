#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "efs.h"

#define MAX_APP_SIZE (1024 * 1024)

static int BLOCK_FILE;
static BlockDevice BLOCK_DEVICE;
static EasyFileSystem EFS;
static Inode ROOT_INODE;

void read_block(BlockCache *b) {
  if (lseek(BLOCK_FILE, b->block_id * BLOCK_SZ, 0) != b->block_id * BLOCK_SZ) {
    perror("lseek");
    exit(1);
  }
  if (read(BLOCK_FILE, b->cache, BLOCK_SZ) != BLOCK_SZ) {
    perror("read");
    exit(1);
  }
}

void write_block(BlockCache *b) {
  if (lseek(BLOCK_FILE, b->block_id * BLOCK_SZ, 0) != b->block_id * BLOCK_SZ) {
    perror("lseek");
    exit(1);
  }
  if (write(BLOCK_FILE, b->cache, BLOCK_SZ) != BLOCK_SZ) {
    perror("read");
    exit(1);
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: mkfs fs.img files...\n");
    exit(1);
  }

  assert((BLOCK_SZ % sizeof(DiskInode)) == 0);
  BLOCK_FILE = open(argv[1], O_RDWR | O_CREAT | O_TRUNC, 0666);
  if (BLOCK_FILE < 0) {
    perror(argv[1]);
    exit(1);
  }

  BlockDevice *block_device = &BLOCK_DEVICE;
  block_device->read_block = read_block;
  block_device->write_block = write_block;

  // 16MiB, at most 4095 files
  EasyFileSystem *efs = &EFS;
  efs_create(efs, block_device, 16 * 2048, 1);

  Inode *root_inode = &ROOT_INODE;
  efs_root_inode(root_inode, efs);

  char *app;
  char all_data[MAX_APP_SIZE];
  FILE *host_file;
  uint64_t host_file_size = 0;
  Inode inode;

  for (uint64_t i = 2; i < argc; i++) {
    // process app name
    app = argv[i];
    for (int64_t i = strlen(app) - 1; i >= 0; i--) {
      if (app[i] == '/') {
        app = app + i + 1;
        break;
      }
    }
    for (int64_t i = 0; i < strlen(app); i++) {
      if (app[i] == '.') {
        app[i] = 0;
        break;
      }
    }

    // load app data from host file system
    host_file = fopen("textfile.txt", "rb");
    fseek(host_file, 0, SEEK_END);
    host_file_size = ftell(host_file);
    assert(host_file_size <= MAX_APP_SIZE);
    fseek(host_file, 0, SEEK_SET);
    fread(all_data, 1, host_file_size, host_file);
    fclose(host_file);

    // create a file in efs
    inode_create(root_inode, app, &inode);

    // write data to efs
    inode_write_at(&inode, 0, (uint8_t *)all_data, host_file_size);
  }

  // list apps
  uint64_t ls_len = inode_ls_len(root_inode);
  char **apps = malloc(ls_len * sizeof(char *));
  for (uint64_t i = 0; i < ls_len; i++) {
    apps[i] = malloc(NAME_LENGTH_LIMIT + 1);
  }
  inode_ls(&ROOT_INODE, apps);
  for (uint64_t i = 0; i < ls_len; i++) {
    printf("%s\n", apps[i]);
  }
  for (uint64_t i = 0; i < ls_len; i++) {
    free(apps[i]);
  }
  free(apps);

  close(BLOCK_FILE);
  return 0;
}
