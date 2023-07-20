/* Copyright (c) 2020 UT Longhorn Racing Solar */

#ifndef __COMMON_H
#define __COMMON_H

#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

/* Used for generating ENUMS */
#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

/**
 * Used for creating getter functions (returns the value based on given inputs) 
 * 
 * GETTER is used for creating the function and EXPOSE_GETTER is used for creating the declaration in the header file
 */
#define GETTER(type, name) \
    type get_##name(void){ \
        return name; \
    } \

#define EXPOSE_GETTER(type, name) \
    type get_##name(void); \

typedef void (*callback_t)(void);

void print_float(char *str, float f);

#endif
