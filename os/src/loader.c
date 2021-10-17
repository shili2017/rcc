#include "config.h"
#include "log.h"
#include "string.h"
#include "task.h"
#include "trap.h"

uint64_t loader_get_num_app() {
  extern uint64_t _num_app;
  return _num_app;
}

uint8_t *loader_get_app_data(uint64_t app_id) {
  extern uint64_t _num_app;
  uint64_t *num_app_ptr = &_num_app;
  uint64_t num_app = loader_get_num_app();

  assert(app_id < num_app, "app_id %llx >= num_app %llx\n", app_id, num_app);

  uint64_t app_start, app_end;
  app_start = num_app_ptr[app_id + 1];
  return (uint8_t *)app_start;
}
