#include <stdio.h>

int logicalShift(int x, int n) {
  int sign = x >> 31;
  sign = ~sign & 0x1;
  int mask = ~(~0 << sign);
  mask = (~0 & mask) >> n;
  mask = mask | (mask + sign);
  return (x >> n) & mask;
}

