#ifndef _ASSERT_H_
#define _ASSERT_H_

#include "stdio.h"
#include "syscall.h"

#define assert(cond)                                                           \
  do {                                                                         \
    if (!(cond)) {                                                             \
      printf("[ASSERT]\n");                                                    \
      exit(-1);                                                                \
    }                                                                          \
  } while (0)

#endif // _ASSERT_H_
