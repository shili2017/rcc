#include <stdint.h>

#include "stdio.h"

#define LEN 100

int main() {
  uint64_t p = 7u;
  uint64_t m = 998244353u;
  uint64_t iter = 160000;
  uint64_t s[LEN];
  uint64_t cur = 0;
  s[cur] = 1;
  for (uint64_t i = 1; i < iter; i++) {
    uint64_t next = (cur + 1 == LEN) ? 0 : (cur + 1);
    s[next] = (s[cur] * p) % m;
    cur = next;
    if (i % 10000 == 0) {
      printf("power_7 [%lld/%lld]\n", i, iter);
    }
  }

  printf("%lld^%lld=%lld\n", p, iter, s[cur]);
  printf("Test power_7 OK!\n");
  return 0;
}
