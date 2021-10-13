#ifndef _BATCH_H_
#define _BATCH_H_

#include <stdint.h>

void batch_init();
void batch_run_next_app();

int check_address_range(uint64_t start, uint64_t end);

#endif // _BATCH_H_
