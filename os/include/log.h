#ifndef _LOG_H_
#define _LOG_H_

#include "sbi.h"
#include "stdio.h"

#define LOG_ERROR 0
#define LOG_WARN 1
#define LOG_INFO 2
#define LOG_DEBUG 3
#define LOG_TRACE 4

#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_INFO
#endif

#define ANSI_RED 31
#define ANSI_YELLOW 93
#define ANSI_BLUE 34
#define ANSI_GREEN 32
#define ANSI_GREY 90

#define LOG_TAG_ERROR "ERROR"
#define LOG_TAG_WARN "WARN"
#define LOG_TAG_INFO "INFO"
#define LOG_TAG_DEBUG "DEBUG"
#define LOG_TAG_TRACE "TRACE"
#define LOG_TAG_PANIC "PANIC"

#define log(level, color, tag, fmt, ...)                                       \
  do {                                                                         \
    if (level <= LOG_LEVEL && level >= LOG_ERROR) {                            \
      printf("\x1b[%dm[%s] " fmt "\x1b[0m", color, tag, ##__VA_ARGS__);        \
    }                                                                          \
  } while (0)

#define error(fmt, ...)                                                        \
  log(LOG_ERROR, ANSI_RED, LOG_TAG_ERROR, fmt, ##__VA_ARGS__)
#define warn(fmt, ...)                                                         \
  log(LOG_WARN, ANSI_YELLOW, LOG_TAG_WARN, fmt, ##__VA_ARGS__)
#define info(fmt, ...)                                                         \
  log(LOG_INFO, ANSI_BLUE, LOG_TAG_INFO, fmt, ##__VA_ARGS__)
#define debug(fmt, ...)                                                        \
  log(LOG_DEBUG, ANSI_GREEN, LOG_TAG_DEBUG, fmt, ##__VA_ARGS__)
#define trace(fmt, ...)                                                        \
  log(LOG_TRACE, ANSI_GREY, LOG_TAG_TRACE, fmt, ##__VA_ARGS__)

#define panic(fmt, ...)                                                        \
  do {                                                                         \
    log(LOG_ERROR, ANSI_RED, LOG_TAG_PANIC, fmt, ##__VA_ARGS__);               \
    shutdown();                                                                \
  } while (0)

#define assert(cond)                                                           \
  do {                                                                         \
    if (!(cond)) {                                                             \
      panic("[ASSERT]\n");                                                     \
    }                                                                          \
  } while (0)

#endif // _LOG_H_
