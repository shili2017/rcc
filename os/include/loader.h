#ifndef _LOADER_H_
#define _LOADER_H_

#include <stdint.h>

#include "task.h"

uint64_t loader_get_num_app();
void loader_load_apps();
TaskContext *loader_init_app_cx(uint64_t app_id);

#endif // _LOADER_H_
