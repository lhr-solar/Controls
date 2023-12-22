/** 
 * @copyright Copyright (c) 2021 UT Longhorn Racing Solar
 * @file ReadTritium.h
 * @brief Reads data from the Tritium Motor Controller
 */

#ifndef __READ_TRITIUM_H
#define __READ_TRITIUM_H

#include "os.h"
#include "common.h"
#include "Tasks.h"

/**
 * @enum tritium_error_code_t
 * @brief Motor Error States
 * Read messages from motor in ReadTritium and trigger appropriate 
 * error messages as needed based on bits
*/
typedef enum{
    /** hardware over current error */
    T_HARDWARE_OVER_CURRENT_ERR = (1<<0), 
    /** software over current error */
    T_SOFTWARE_OVER_CURRENT_ERR = (1<<1),
    /** DC bus over voltage error */
    T_DC_BUS_OVERVOLT_ERR = (1<<2),
    /** hall sensor error */
    T_HALL_SENSOR_ERR = (1<<3), 
    /** watchdog last reset error */
    T_WATCHDOG_LAST_RESET_ERR = 
    /** config read error */(1<<4), 
    T_CONFIG_READ_ERR = (1<<5), 
    /** under voltage lockout error */
    T_UNDER_VOLTAGE_LOCKOUT_ERR = (1<<6),
    /** desaturation fault error */
    T_DESAT_FAULT_ERR = (1<<7), 
    /** motor over speed error */
    T_MOTOR_OVER_SPEED_ERR = (1<<8),
    /** motor controller fails to restart or initialize */ = (1<<8), 
    T_INIT_FAIL = (1<<9), 
    /** motor watchdog trip */
    T_MOTOR_WATCHDOG_TRIP = (1<<15),
    /** no error */ 
    T_NONE = 0x00,
} tritium_error_code_t;

/**
 * @brief Gets the motor velocity in RPM
 * @return float Motor velocity in RPM
 */
float Motor_RPM_Get();

/**
 * @brief Gets the motor velocity in m/s
 * @return float Motor velocity in m/s
*/
float Motor_Velocity_Get();

#endif
