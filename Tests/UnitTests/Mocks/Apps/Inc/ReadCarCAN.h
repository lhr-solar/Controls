/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#ifdef TEST_READCARCAN
#include_next "ReadCarCAN.h"
#else

#ifndef __READ_CAR_CAN_H
#define __READ_CAR_CAN_H

#include "fff.h"
#include "common.h"

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


DECLARE_FAKE_VOID_FUNC(Task_ReadCarCAN, void*);

DECLARE_FAKE_VALUE_FUNC(bool, ChargeEnable_Get);

#endif
#endif
