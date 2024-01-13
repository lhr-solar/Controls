/** 
 * @file ReadTritium.h
 * @brief Reads messages sent by the Tritium motor controller & handles conditional logic based on the message.
 * 
 * Call Motor_RPM_Get() to get the motor velocity in RPM. Call 
 * Motor_Velocity_Get() to get the motor velocity in m/s. Starting the task will
 * automatically forward all messages from MotorCAN to CarCAN, detect and handle motor errors,
 * and forward velocity information to the display.
 * 
 */

#ifndef __READ_TRITIUM_H
#define __READ_TRITIUM_H

#include "os.h"
#include "common.h"
#include "Tasks.h"

/**
 * Motor Error States
*/
typedef enum{
    /** Hardware over current error */
    T_HARDWARE_OVER_CURRENT_ERR = (1<<0), 
    /** Software over current error */
    T_SOFTWARE_OVER_CURRENT_ERR = (1<<1),
    /** DC bus over voltage error */
    T_DC_BUS_OVERVOLT_ERR = (1<<2),
    /** Hall sensor error */
    T_HALL_SENSOR_ERR = (1<<3), 
    /** Watchdog last reset error */
    T_WATCHDOG_LAST_RESET_ERR = (1<<4), 
    /** Config read error */
    T_CONFIG_READ_ERR = (1<<5), 
    /** Under voltage lockout error */
    T_UNDER_VOLTAGE_LOCKOUT_ERR = (1<<6),
    /** Desaturation fault error */
    T_DESAT_FAULT_ERR = (1<<7), 
    /** Motor over speed error */
    T_MOTOR_OVER_SPEED_ERR = (1<<8),
    /** Motor controller fails to restart or initialize */
    T_INIT_FAIL = (1<<9), 
    /** Motor watchdog trip */
    T_MOTOR_WATCHDOG_TRIP = (1<<15),
    /** No error */ 
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
