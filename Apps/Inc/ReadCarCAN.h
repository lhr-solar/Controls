/* Copyright (c) 2020 UT Longhorn Racing Solar */

#ifndef __READ_CAR_CAN_H
#define __READ_CAR_CAN_H

#include "os.h"
#include "common.h"
#include "Tasks.h"
#include "CANbus.h"

#define SAT_BUF_LENGTH 5 

// Precharge Delay times in seconds
#define PRECHARGE_MOTOR_DELAY 1     // 7 Seconds, need to change
#define PRECHARGE_ARRAY_DELAY 1     // 2 Seconds, need to change

/**
 * Error types
 */
typedef enum{
	READCARCAN_ERR_NONE			    = 0,
	READCARCAN_ERR_CHARGE_DISABLE   =-1,	// Received a charge disable msg
	READCARCAN_ERR_MISSED_MSG		=-2,	// Didn't receive a BPS charge msg in time
	READCARCAN_ERR_BPS_TRIP		    =-3		// Received a BPS trip msg (0 or 1)
} ReadCarCAN_error_code_t;

/**
 * @brief Returns whether regen braking / charging is enabled or not
 * @return  Whether regen braking / charging is enabled or not
*/
bool ChargeEnable_Get(void);

/**
 * @brief Returns the value of charge message saturation
 * @return Value between -15 to 15
*/
int8_t ChargeMsgSaturation_Get(void);

/**
 * @brief Returns the value of charge message saturation
 * @return Whether array ignition is ON/OFF
*/
bool ArrayIgnitionStatus_Get(void);

/**
 * @brief Returns the value of state of charge
 * @return Value between 1 to 100
*/
uint8_t SOC_Get(void);

/**
 * @brief Returns the value of supply voltage
 * @return Voltage in mV
*/
uint32_t SBPV_Get(void);

// Getter function for array ignition status
bool PreChargeComplete_Get(void);

#endif