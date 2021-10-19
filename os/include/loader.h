#ifndef _LOADER_H_
#define _LOADER_H_

#include <stddef.h>
#include <stdint.h>

uint64_t loader_get_num_app();
uint8_t *loader_get_app_data(uint64_t app_id);
size_t loader_get_app_size(uint64_t app_id);

#endif // _LOADER_H_
