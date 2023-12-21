/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file ReadCarCAN.h
 * @brief The Read Car CAN task reads the car’s CANBus for values 
 * relevant to Controls. Currently we parse four types of messages: 
 * BPS trip, BPS contactor state, Supplemental Voltage, and State of Charge.
 */

#ifndef __READ_CAR_CAN_H
#define __READ_CAR_CAN_H

#include "os.h"
#include "common.h"
#include "Tasks.h"
#include "CANbus.h"

/**
 * @def SAT_BUF_LENGTH
 * @brief Length of the array and motor PBC saturation buffers
 */
#define SAT_BUF_LENGTH 5 

/**
 * @def ARRAY_SATURATION_THRESHOLD
 * @brief The Array Saturation Threshold is used to determine if Controls has 
 *          received a sufficient number of BPS's HV Array Enable Messages.
 *          BPS Array Saturation threshold is halfway between 0 and max saturation value.
 */
#define ARRAY_SATURATION_THRESHOLD (((SAT_BUF_LENGTH + 1) * SAT_BUF_LENGTH) / 4) 

/**
 * @def PLUS_MINUS_SATURATION_THRESHOLD
 * @brief The Plus-Minus Saturation Threshold is used to determine if Controls has
 *         received a sufficient number of BPS's HV Plus-Minus Enable Messages.
 *        BPS Plus-Minus Saturation threshold is halfway between 0 and max saturation value.
 */
*/
#define PLUS_MINUS_SATURATION_THRESHOLD (((SAT_BUF_LENGTH + 1) * SAT_BUF_LENGTH) / 4) 

/**
 * @def CAN_WATCH_TMR_DLY_MS
 * @brief Delay time in milliseconds for the CAN Watchdog Timer
 */
#define CAN_WATCH_TMR_DLY_MS 500u // 500 ms

/**
 * @def CAN_WATCH_TMR_DLY_TMR_TS
 * @brief Delay time in timer ticks for the CAN Watchdog Timer
 */
#define CAN_WATCH_TMR_DLY_TMR_TS ((CAN_WATCH_TMR_DLY_MS * OS_CFG_TMR_TASK_RATE_HZ) / (1000u))

/**
 * @def PRECHARGE_MOTOR_DELAY
 * @brief Delay time in milliseconds for the Motor Precharge.
 */
#define PRECHARGE_MOTOR_DELAY 100u

/**
 * @def MOTOR_CONTROLLER_PRECHARGE_BYPASS_DLY_TMR_TS
 * @brief Delay time in timer ticks for the Motor Precharge Bypass Timer
*/
#define MOTOR_CONTROLLER_PRECHARGE_BYPASS_DLY_TMR_TS ((PRECHARGE_MOTOR_DELAY * OS_CFG_TMR_TASK_RATE_HZ) / (1000u))

/**
 * @def PRECHARGE_ARRAY_DELAY
 * @brief Delay time in milliseconds for the Array Precharge.
 */
#define PRECHARGE_ARRAY_DELAY 100u

/**
 * @def ARRAY_PRECHARGE_BYPASS_DLY_TMR_TS
 * @brief Delay time in timer ticks for the Array Precharge Bypass Timer
 */
#define ARRAY_PRECHARGE_BYPASS_DLY_TMR_TS ((PRECHARGE_ARRAY_DELAY * OS_CFG_TMR_TASK_RATE_HZ) / (1000u)) 

/**
 * @enum ReadCarCAN_error_code_t
 * @brief Error codes for ReadCarCAN
 * 
 */
typedef enum{
    /** No error */
	READCARCAN_ERR_NONE,
    /** Received a charge disable msg **/	    		
	READCARCAN_ERR_CHARGE_DISABLE,
	/** Didn't receive a BPS charge msg in time **/
    READCARCAN_ERR_MISSED_MSG,
    /** Ignition is turned to neither (off due to LV) or both at the same time (impossible) are on at  **/
	READCARCAN_ERR_DISABLE_CONTACTORS_MSG,
	/** Received a BPS trip msg (0 or 1) **/
    READCARCAN_ERR_BPS_TRIP
} ReadCarCAN_error_code_t;

/**
 * @brief Returns whether regen braking / charging is enabled or not
 * @return true if regen braking / charging is enabled, false otherwise
 */
*/
bool ChargeEnable_Get(void);

#endif



