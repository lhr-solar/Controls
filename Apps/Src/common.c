#include "common.h"

void print_float(char * str, float f) {
    if(str) printf("%s", str);

    int32_t n = (int32_t)f;
    f -= n;
    f *= 100;
    int32_t d = (f<0)?-f:f;
    printf("%d.%02d\n\r", (int)n, (int)d);
}

void print_bin(char * str, uint32_t i){
    if(str) printf("%s", str);

    for(uint32_t mask=0x80000000L; mask > 0L; mask >>= 1){
        printf("%d\n\r",(mask & i)?1:0);
    }
}