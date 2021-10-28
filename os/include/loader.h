#ifndef _LOADER_H_
#define _LOADER_H_

#include <stddef.h>
#include <stdint.h>

uint64_t loader_get_num_app();
uint8_t *loader_get_app_data(uint64_t app_id);
size_t loader_get_app_size(uint64_t app_id);
void loader_init_and_list_apps();
uint8_t *loader_get_app_data_by_name(char *name);
size_t loader_get_app_size_by_name(char *name);

#endif // _LOADER_H_
