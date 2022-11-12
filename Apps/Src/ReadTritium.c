/* Copyright (c) 2021 UT Longhorn Racing Solar */

#include "ReadTritium.h"
#include "CAN_Queue.h"
#include "CANbus.h"
#include <string.h>

//status limit flag masks
#define MASK_MOTOR_TEMP_LIMIT (1<<6) //check if motor temperature is limiting the motor 6

uint16_t Motor_FaultBitmap = T_NONE;


/**
 * @brief Return the static error field from this layer
 * 
 */
tritium_error_code_t MotorController_getTritiumError(void){
    //TODO: implement for loop to parse this
	for(int i = 0; i < NUM_TRITIUM_ERRORS; ++i){
		if(Motor_FaultBitmap & (1<<i)){
			return ((tritium_error_code_t)(1<<i));
		}
	}
}

/**
 * @brief   Assert Error if Tritium sends error. When Fault Bitmap is set,
 *          and semaphore is posted, Fault state will run.
 * @param   motor_err Bitmap which has motor error codes
 */
static void assertTritiumError(uint16_t motor_err){    
	OS_ERR err;
	if(motor_err != T_NONE){
		FaultBitmap |= FAULT_TRITIUM;
		OSSemPost(&FaultState_Sem4, OS_OPT_POST_1, &err);
		assertOSError(0, err);
	}
}


/* OBJECTIVES:
Objective 1:
- Receive motor status message from MotorController (18.4.2)
- interpret error status
	- if error
		- assertOSError
- determine information important to telementry
	- SendCarCAN
- determine information important for storage
	- acquire mutex on Storage Array
	- Store information in Storage Array (based on index) 
	- release mutex on Storage Array

Objective 2:
- create function able to read data from Storage Array
	- pend on Storage Array mutex
	- acquire Storage Array mutex
	- read information of array index 
	- release Storage Array mutex
*/

void Task_ReadTritium(void *p_arg){
	OS_ERR err;
	CANDATA_t dataBuf = {0};
	
	while (1){
		ErrorStatus status = CANbus_Read(&dataBuf, CAN_BLOCKING, MOTORCAN);

		if (status == SUCCESS){
			switch(dataBuf.ID){
				case MOTOR_STATUS:{
					// motor status error flags is in bytes 4-5
					uint16_t errorFlags = *((uint16_t*)(&dataBuf.data[4]));
					Motor_FaultBitmap = errorFlags;
					assertTritiumError(errorFlags);
					break;
				}
				default:{
					break; //for cases not handled currently
				}

			}


		}

		OSTimeDlyHMSM(0, 0, 0, 10, OS_OPT_TIME_HMSM_NON_STRICT, &err);
		assertOSError(OS_READ_TRITIUM_LOC, err);
	}
}