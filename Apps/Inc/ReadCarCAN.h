/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file ReadCarCAN.h
 * @brief 
 * 
 * @addtogroup ReadCarCAN
 * @{
 */

#ifndef __READ_CAR_CAN_H
#define __READ_CAR_CAN_H

#include "os.h"
#include "common.h"
#include "Tasks.h"
#include "CANbus.h"

// Length of the array and motor PBC saturation buffers
#define SAT_BUF_LENGTH 5 

// The Array/Motor Controller Saturation Threshold is used to determine if Controls has 
//      received a sufficient number of BPS's HV Array/Plus-Minus Enable Messages.
//      BPS Array and Plus/Minus saturation threshold is halfway between 0 and max saturation value.
#define ARRAY_SATURATION_THRESHOLD (((SAT_BUF_LENGTH + 1) * SAT_BUF_LENGTH) / 4) 
#define PLUS_MINUS_SATURATION_THRESHOLD (((SAT_BUF_LENGTH + 1) * SAT_BUF_LENGTH) / 4) 

// Timer delay constants
#define CAN_WATCH_TMR_DLY_MS 500u // 500 ms
#define CAN_WATCH_TMR_DLY_TMR_TS ((CAN_WATCH_TMR_DLY_MS * OS_CFG_TMR_TASK_RATE_HZ) / (1000u)) // 1000 for ms -> s conversion

// Precharge Delay times in milliseconds
#define PRECHARGE_PLUS_MINUS_DELAY 100u	// 100 ms, as this the smallest time delay that the RTOS can work with
#define PRECHARGE_ARRAY_DELAY 100u     	// 100 ms
#define ARRAY_PRECHARGE_BYPASS_DLY_TMR_TS ((PRECHARGE_ARRAY_DELAY * OS_CFG_TMR_TASK_RATE_HZ) / (1000u)) 
#define MOTOR_CONTROLLER_PRECHARGE_BYPASS_DLY_TMR_TS ((PRECHARGE_PLUS_MINUS_DELAY * OS_CFG_TMR_TASK_RATE_HZ) / (1000u))

/**
 * Error types
 */
typedef enum{
	READCARCAN_ERR_NONE,			    		
	READCARCAN_ERR_CHARGE_DISABLE,   			// Received a charge disable msg
	READCARCAN_ERR_MISSED_MSG,					// Didn't receive a BPS charge msg in time
	READCARCAN_ERR_DISABLE_CONTACTORS_MSG,   	// Ignition is turned to neither (off due to LV) or both at the same time (impossible) are on at 
	READCARCAN_ERR_BPS_TRIP		    		    // Received a BPS trip msg (0 or 1)
} ReadCarCAN_error_code_t;

/**
 * @brief Returns whether regen braking / charging is enabled or not
 * @return  Whether regen braking / charging is enabled or not
*/
bool ChargeEnable_Get(void);

#endif


/** @} */
