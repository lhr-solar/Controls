/**
 * @file ReadCarCAN.h
 * @brief Reads the car’s CAN bus for values relevant to Controls, and handles ignition sequence logic.
 * 
 * Call ChargeEnable_Get() to get whether regen braking / charging is enabled or not. Starting the task will
 * spawn the watchdog, handle the ignition sequence logic in communication with BPS, and update the display with
 * the State of Charge and the Supplemental Voltage of the car.
 * 
 */

#ifndef __READ_CAR_CAN_H
#define __READ_CAR_CAN_H

#include "os.h"
#include "common.h"
#include "Tasks.h"
#include "CANbus.h"

/**
 * Length of the array and motor precharge-bypass contactor (PBC) saturation buffers
 */
#define SAT_BUF_LENGTH 5 

/**
 * The Array Saturation Threshold is used to determine if Controls has 
 * received a sufficient number of BPS's HV Array Enable Messages.
 * BPS Array Saturation threshold is halfway between 0 and max saturation value.
 */
#define ARRAY_SATURATION_THRESHOLD (((SAT_BUF_LENGTH + 1) * SAT_BUF_LENGTH) / 4) 

/**
 * The Plus-Minus Saturation Threshold is used to determine if Controls has
 * received a sufficient number of BPS's HV Plus-Minus Enable Messages.
 * BPS Plus-Minus Saturation threshold is halfway between 0 and max saturation value.
 */
#define PLUS_MINUS_SATURATION_THRESHOLD (((SAT_BUF_LENGTH + 1) * SAT_BUF_LENGTH) / 4) 

/**
 * Delay time in milliseconds for the CAN Watchdog Timer
 */
#define CAN_WATCH_TMR_DLY_MS 500u

/**
 * Delay time in milliseconds for the Motor Precharge.
 */
#define PRECHARGE_MOTOR_DELAY 100u

/**
 * Delay time in milliseconds for the Array Precharge.
 */
#define PRECHARGE_ARRAY_DELAY 100u

/**
 * Error codes for ReadCarCAN
 */
typedef enum {
    /** No error */
	READCARCAN_ERR_NONE,
    /** Received a charge disable msg **/	    		
	READCARCAN_ERR_CHARGE_DISABLE,
	/** Didn't receive a BPS charge msg in time **/
    READCARCAN_ERR_MISSED_MSG,
    /** Ignition is turned to neither (off due to LV) or both are on at the same time (impossible)  **/
	READCARCAN_ERR_DISABLE_CONTACTORS_MSG,
	/** Received a BPS trip msg (0 or 1) **/
    READCARCAN_ERR_BPS_TRIP
} ReadCarCAN_error_code_t;

/**
 * @brief Returns whether regen braking / charging is enabled or not
 * @return true if regen braking / charging is enabled, false otherwise
 */
bool ChargeEnable_Get(void);

#endif



