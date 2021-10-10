#include "log.h"
#include "printf.h"

static const unsigned log_color_table[5] = {
  ANSI_RED,
  ANSI_YELLOW,
  ANSI_BLUE,
  ANSI_GREEN,
  ANSI_GREY
};

static const char* log_string_table[5] = {
  "ERROR",
  "WARN",
  "INFO",
  "DEBUG",
  "TRACE"
};

int panic(const char* fmt, ...) {
  va_list va;
  va_start(va, fmt);
  int ret = printf("[PANIC] ");
  ret |= printf(fmt, va);
  va_end(va);
  return ret;
}

int log(unsigned level, const char* fmt, ...) {
  if (level > LOG_LEVEL)
    return 0;
  
  if (level > LOG_TRACE || level < LOG_ERROR)
    return -1;

  va_list va;
  va_start(va, fmt);
  int ret = printf("\x1b[%dm", log_color_table[level]);
  ret |= printf("[%s] ", log_string_table[level]);
  ret |= printf(fmt, va);
  ret |= printf("\x1b[0m");
  va_end(va);
  return ret;
}
