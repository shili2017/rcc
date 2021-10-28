#include "config.h"
#include "log.h"
#include "string.h"

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

static char APP_NAMES[MAX_APP_NUM][MAX_APP_NAME_LENGTH];

void loader_init_and_list_apps() {
  extern uint64_t _app_names;
  uint64_t num_app = loader_get_num_app();
  uint8_t *ptr = (uint8_t *)&_app_names;

  info("/**** APPS ****\n");
  for (uint64_t i = 0; i < num_app; i++) {
    strcpy(APP_NAMES[i], (char *)ptr);
    ptr += (strlen((char *)ptr) + 1);
    info("%s\n", APP_NAMES[i]);
  }
  info("**************/\n");
}

uint8_t *loader_get_app_data_by_name(char *name) {
  uint64_t num_app = loader_get_num_app();
  for (uint64_t i = 0; i < num_app; i++) {
    if (strcmp(APP_NAMES[i], name) == 0) {
      return loader_get_app_data(i);
    }
  }
  return NULL;
}

size_t loader_get_app_size_by_name(char *name) {
  uint64_t num_app = loader_get_num_app();
  for (uint64_t i = 0; i < num_app; i++) {
    if (strcmp(APP_NAMES[i], name) == 0) {
      return loader_get_app_size(i);
    }
  }
  return 0;
}
