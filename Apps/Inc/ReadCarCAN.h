/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file ReadCarCAN.h
 * @brief 
 * 
 * @defgroup ReadCarCAN
 * @addtogroup ReadCarCAN
 * @{
 */

#ifndef __READ_CAR_CAN_H
#define __READ_CAR_CAN_H

#include "os.h"
#include "common.h"
#include "Tasks.h"
#include "CANbus.h"

/**
 * Error types
 */
typedef enum{
	READCARCAN_ERR_NONE			    		= 0,
	READCARCAN_ERR_CHARGE_DISABLE   		=-1,	// Received a charge disable msg
	READCARCAN_ERR_MISSED_MSG				=-2,	// Didn't receive a BPS charge msg in time
	READCARCAN_ERR_DISABLE_CONTACTORS_MSG   =-3,		// Ignition is turned to neither (off due to LV) or both at the same time (impossible) are on at 
	READCARCAN_ERR_BPS_TRIP		    		=-4		// Received a BPS trip msg (0 or 1)
} ReadCarCAN_error_code_t;

/**
 * @brief Returns whether regen braking / charging is enabled or not
 * @return  Whether regen braking / charging is enabled or not
*/
bool ChargeEnable_Get(void);

#endif


/* @} */
