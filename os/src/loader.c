#include "config.h"
#include "log.h"

uint64_t loader_get_num_app() {
  extern uint64_t _num_app;
  return _num_app;
}

uint8_t *loader_get_app_data(uint64_t app_id) {
  uint64_t num_app = loader_get_num_app();
  assert(app_id < num_app, "app_id %llx >= num_app %llx\n", app_id, num_app);

  extern uint64_t _num_app;
  uint64_t *num_app_ptr = &_num_app;
  num_app_ptr++;

  return (uint8_t *)num_app_ptr[app_id];
}

size_t loader_get_app_size(uint64_t app_id) {
  uint64_t num_app = loader_get_num_app();
  assert(app_id < num_app, "app_id %llx >= num_app %llx\n", app_id, num_app);

  extern uint64_t _num_app;
  uint64_t *num_app_ptr = &_num_app;
  num_app_ptr++;

  return (size_t)(num_app_ptr[app_id + 1] - num_app_ptr[app_id]);
}
