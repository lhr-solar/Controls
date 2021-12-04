/* Copyright (c) 2020 UT Longhorn Racing Solar */

/**
 * Configuration file to hold
 * shared constants, enumerations, structs
 */

#ifndef __CONFIG_H
#define __CONFIG_H

typedef enum {OFF = 0, ON} State;

// Wheel diameter in meters
#define WHEEL_DIAMETER 0.4f
// Any sort of gear reduction from the motor to the wheels
// Defined as motor velocity / wheel velocity
#define GEAR_REDUCTION 1.0f

#define PRECHARGE_ARRAY_DELAY 5

#ifndef __STM32F4xx_H
typedef enum {ERROR = 0, SUCCESS = !ERROR} ErrorStatus;
#endif

#endif
