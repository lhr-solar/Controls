#include "common.h"

void print_float(char * str, float f) {
    if(str) printf(str);

    int n = (int) f;
    f -= n;
    f *= (10 << 2);
    printf("%d.%d\n\r", n, (int) f);
}
