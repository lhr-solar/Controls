/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file common.h
 * @brief 
 * 
 * @defgroup common
 * @addtogroup common
 * @{
 */

#ifndef __COMMON_H
#define __COMMON_H

#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "config.h"

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

/* Used for generating ENUMS */
#define GENERATE_ENUM(ENUM) ENUM
#define GENERATE_STRING(STRING) #STRING

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

/**
 * Used for creating setter functions (sets the value based on given inputs) 
 * 
 * SETTER is used for creating the function and EXPOSE_SETTER is used for creating the declaration in the header file
 */
#define SETTER(type, name) \
    void set_##name(type val){ \
        name = val; \
    } \

#define EXPOSE_SETTER(type, name) \
    void set_##name(type val); \

typedef void (*callback_t)(void);

void print_float(char *str, float f);

/**
 * @brief Meters per second to rpm conversion
 * @param velocity_mps velocity in meters per second
 * @returns rpm
*/
inline float mpsToRpm(float velocity_mps){
    return (velocity_mps * 60) / WHEEL_CIRCUMFERENCE;
}

#endif

/* @} */
