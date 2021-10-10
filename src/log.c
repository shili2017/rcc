#include "printf.h"

int panic(const char* format, ...) {
  va_list va;
  va_start(va, format);
  const int ret = printf(format, va);
  va_end(va);
  return ret;
}
