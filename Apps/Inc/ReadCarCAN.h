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

#define BPS_TRIP_MESSAGE 1

/**
 * Error types
 */
typedef enum
{
	READCARCAN_ERR_NONE = 0x0000,
	//READCARCAN_ERR_CHARGE_DISABLE = 0xAAAA,		    // Received a charge disable msg
	READCARCAN_ERR_BPS_MISSED_MSG = 0xBBBB,			    // Didn't receive a BPS charge msg in time
	READCARCAN_ERR_PCHG_MISSED_MSG = 0xCCCC,			    // Didn't receive a precharge msg in time
	READCARCAN_ERR_ACTIVEPRECHARGE_TMOUT_MOTOR = 0xCCCA,
	READCARCAN_ERR_ACTIVEPRECHARGE_TMOUT_ARR = 0xCCCB,
	READCARCAN_ERR_BPS_TRIP = 0xDDDD,				// Received a BPS trip msg (0 or 1)
	READCARCAN_ERR_ACTIVE_PRECHARGE_FAULT = 0xEEEE,  // Active precharge sent a fault message
} ReadCarCAN_error_code_t;


void assertReadCarCANError(ReadCarCAN_error_code_t rcc_err);

#endif

/* @} */
