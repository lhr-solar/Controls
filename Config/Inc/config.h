/* Copyright (c) 2020 UT Longhorn Racing Solar */

/**
 * Configuration file to hold
 * shared constants, enumerations, structs
 */

#ifndef __CONFIG_H
#define __CONFIG_H

typedef enum {OFF = 0, ON} State;

// Wheel diameter in meters
#define WHEEL_DIAMETER 0.5504f

#define PRECHARGE_MOTOR_DELAY 7     // 7 Seconds
#define PRECHARGE_ARRAY_DELAY 2     // 2 Seconds

// Used by the system for errors
#ifndef __STM32F4xx_H
typedef enum {ERROR = 0, SUCCESS = !ERROR} ErrorStatus;
#endif

#endif
