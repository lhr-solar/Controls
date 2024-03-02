/* Copyright (c) 2020 UT Longhorn Racing Solar */

/**
 * Configuration file to hold
 * shared constants, enumerations, structs
 */

#ifndef __CONFIG_H
#define __CONFIG_H

typedef enum {OFF = 0, ON} State;

// Wheel diameter in meters
#define WHEEL_CIRCUMFERENCE 3.35145104  // m

// Used by the system for errors
#ifndef __STM32F4xx_H
typedef enum {ERROR = 0, SUCCESS = !ERROR} ErrorStatus;
#endif

#endif
