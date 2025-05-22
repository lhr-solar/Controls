/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file SendTritium.h
 * @brief 
 * 
 * @defgroup SendTritium
 * @addtogroup SendTritium
 * @{
 */
#ifndef __SENDTRITIUM_H
#define __SENDTRITIUM_H

#include "common.h"
#include "Dashboard.h"

//#define SENDTRITIUM_PRINT_MES

#define MOTOR_MSG_PERIOD 100 // in ms
#define FSM_PERIOD 100 // in ms
#define DEBOUNCE_PERIOD 2 // in units of FSM_PERIOD

#define MAX_VELOCITY 20000.0f // rpm (unobtainable value)

// Used to define accel & brake (hysteresis) thresholds for when to start/stop powering the motor, respectively
// TODO: Test these thresholds
#define ACCEL_PEDAL_THRESHOLD 15 // percent
#define BRAKE_UNPRESSED_THRESHOLD 15 // percent
#define BRAKE_PRESSED_THRESHOLD 25 // percent

// Motor Controller current values. Current is in Amps (A)
#define MAX_MOCO_BATTERY_CURRENT 64.0f  // NOTE: Provided only for reference. This 64A max for daybreak, anticipated to be 135 for next-gen
#define CONT_MOCO_BATTERY_CURRENT 30.0f // Continuous 
#define MAX_MOCO_CURRENT 122.0f

#define PEDAL_MIN 0        // percent
#define PEDAL_MAX 100      // percent
#define CURRENT_SP_MIN 0   // percent
#define CURRENT_SP_MAX 100 // percent

#define GEAR_FAULT_THRESHOLD 3 // number of times gear fault can occur before it is considered a fault

/**
 * Error types
 * 
 */
typedef enum
{
    SENDTRITIUM_ERR_NONE,
    SENDTRITIUM_ERR_GEAR_FAULT,     // Received multiple or no gear inputs (e.g. FOR_SW, REV_SW)
} SendTritium_error_code_t;

#ifdef SENDTRITIUM_EXPOSE_VARS
// Inputs
extern uint8_t brakePedalPercent;
extern uint8_t accelPedalPercent;
extern gear_t gear;
#endif

// Getter functions for local variables in SendTritium.c
EXPOSE_GETTER(uint8_t, brakePedalPercent)
EXPOSE_GETTER(uint8_t, accelPedalPercent)
EXPOSE_GETTER(gear_t, gear)
EXPOSE_GETTER(float, currentSetpoint)
EXPOSE_GETTER(float, velocitySetpoint)

/**
 * @brief Linearly map range of integers to another range of integers, and provide the pecentage result.
 * in_min to in_max is mapped to out_min to out_max.
 * @param input input integer value
 * @param in_min minimum value of input range
 * @param in_max maximum value of input range
 * @param out_min minimum value of output range
 * @param out_max maximum value of output range
 * @returns float value from (out_min / 100.0) to (out_max / 100.0)
 */
float mapToPercent(uint8_t input, uint8_t in_min, uint8_t in_max, uint8_t out_min, uint8_t out_max);

#endif

/* @} */
