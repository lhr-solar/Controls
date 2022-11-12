/* Copyright (c) 2021 UT Longhorn Racing Solar */

#ifndef __READ_TRITIUM_H
#define __READ_TRITIUM_H

#include "os.h"
#include "common.h"
#include "Tasks.h"

/**
 * Motor Error States
 * Read messages from motor in ReadTritium and trigger appropriate error messages as needed based on bits
 * 
 */
typedef enum{
    T_HARDWARE_OVER_CURRENT_ERR = (1<<0), 
    T_SOFTWARE_OVER_CURRENT_ERR = (1<<1), 
    T_DC_BUS_OVERVOLT_ERR = (1<<2), 
    T_HALL_SENSOR_ERR = (1<<3), 
    T_WATCHDOG_LAST_RESET_ERR = (1<<4), 
    T_CONFIG_READ_ERR = (1<<5), 
    T_UNDER_VOLTAGE_LOCKOUT_ERR = (1<<6), 
    T_DESAT_FAULT_ERR = (1<<7), 
    T_MOTOR_OVER_SPEED_ERR = (1<<8), 
    T_NONE = 0x00,
} tritium_error_code_t;
#define NUM_TRITIUM_ERRORS 9 //9 errors, and 1 entry for no error

/**
 * @brief Returns the current error status of the tritium controller
 */
tritium_error_code_t MotorController_getTritiumError(void);

#endif