/**
 * @file common.c
 * 
*/

#include "common.h"

#define MASK 0x80000000L

void PrintFloat(char* str, float value) {
    if (str) {
        printf("%s", str);
    }

    int32_t num = (int32_t)value;
    value -= (float)num;
    value *= 100;
    int32_t dec = (int32_t)((value < 0) ? -value : value);
    printf("%d.%02d\n\r", (int)num, (int)dec);
}

void PrintBin(char* str, uint32_t value) {
    if (str) {
        printf("%s", str);
    }

    for (uint32_t mask = MASK; mask > 0L; mask >>= 1) {
        printf("%d\n\r", (mask & value) ? 1 : 0);
    }
}