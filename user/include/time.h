#ifndef _TIME_H_
#define _TIME_H_

#define USEC_PER_SEC 1000000

struct TimeVal {
  uint64_t sec;
  uint64_t usec;
};

typedef struct TimeVal TimeVal;

#endif // _TIME_H_
