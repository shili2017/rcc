#include "stdio.h"

#define SIZE 10
#define P 3
#define STEP 100000
#define MOD 10007

int main() {
  unsigned pow[SIZE];
  unsigned index = 0;
  pow[index] = 1;

  for (unsigned i = 1; i < STEP; i++) {
    unsigned last = pow[index];
    index = (index + 1) % SIZE;
    pow[index] = (last * P) % MOD;
    if (i % 10000 == 0) {
      printf("%d^%d=%d\n", P, i, pow[index]);
    }
  }

  printf("Test power OK!\n");
  return 0;
}
