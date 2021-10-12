#include <stdint.h>

#include "batch.h"
#include "fs.h"
#include "log.h"

int64_t sys_exit(int exit_code) {
  info("Application exited with code %d\n", exit_code);
  batch_run_next_app();
  return 0;
}
