/**
 * @file common.h
 * @brief
 *
 */

#ifndef COMMON_H
#define COMMON_H

#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"

/* Used for generating ENUMS */
#define GENERATE_ENUM(ENUM) ENUM
#define GENERATE_STRING(STRING) #STRING

typedef void (*Callback)(void);

void PrintFloat(char *str, float value);

#define SEC_PER_MIN 60

/**
 * @brief Meters per second to rpm conversion
 * @param velocity_mps velocity in meters per second
 * @returns rpm
 */
inline float MpsToRpm(float velocity_mps) {
    return (float)((velocity_mps * SEC_PER_MIN) / WHEEL_CIRCUMFERENCE);
}

#endif
