#include "common.h"

void print_float(char* str, float f) {
  if (str) printf(str);

  int32_t n = (int32_t)f;
  f -= n;
  f *= 100;
  int32_t d = (f < 0) ? -f : f;
  printf("%d.%02d\n\r", (int)n, (int)d);
}
