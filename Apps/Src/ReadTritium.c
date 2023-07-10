/* Copyright (c) 2021 UT Longhorn Racing Solar */

#include "ReadTritium.h"
#include "CANbus.h"
#include "UpdateDisplay.h"
#include <string.h>

//status limit flag masks
#define MASK_MOTOR_TEMP_LIMIT (1<<6) //check if motor temperature is limiting the motor 6
#define MAX_CAN_LEN 8


uint16_t Motor_FaultBitmap = T_NONE;
static float Motor_RPM = CAR_STOPPED; //Car is stopped until velocity is read
static float Motor_Velocity = CAR_STOPPED; //^^^^

/**
 * @brief Returns highest priority tritium error code
 * 
 */
tritium_error_code_t MotorController_getTritiumError(void){
    //TODO: implement for loop to parse this
	for(int i = 0; i < NUM_TRITIUM_ERRORS; ++i){
		if(Motor_FaultBitmap & (1<<i)){
			return ((tritium_error_code_t)(1<<i));
		}
	}
	return T_NONE;
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
	- Telemetry
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
		ErrorStatus status = CANbus_Read(&dataBuf, true, MOTORCAN);

		if (status == SUCCESS){
			switch(dataBuf.ID){
				case MOTOR_STATUS:{
					// motor status error flags is in bytes 4-5
					Motor_FaultBitmap = *((uint16_t*)(&dataBuf.data[4])); //Storing error flags into Motor_FaultBitmap
					assertTritiumError(Motor_FaultBitmap);
					break;
				}
				
				case VELOCITY:{
                    memcpy(&Motor_RPM, &dataBuf.data[0], sizeof(float));
                    memcpy(&Motor_Velocity, &dataBuf.data[4], sizeof(float));

					//Motor RPM is in bytes 0-3
					Motor_RPM = *((float*)(&dataBuf.data[0]));

					//Car Velocity (in m/s) is in bytes 4-7
					Motor_Velocity = *((float*)(&dataBuf.data[4]));
					uint32_t Car_Velocity = Motor_Velocity;
					
					Car_Velocity = ((Car_Velocity * 100) * 3600); //Converting from m/s to m/h, using fixed point factor of 100
					Car_Velocity = ((Car_Velocity / 160934) * 10); //Converting from m/h to mph, multiplying by 10 to make value "larger" for displaying

					UpdateDisplay_SetVelocity(Car_Velocity);

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

void MotorController_Restart(void){
	CANDATA_t resetmsg = {0};
	resetmsg.ID = MOTOR_RESET;
	CANbus_Send(resetmsg, true, MOTORCAN);
}


float Motor_RPM_Get(){ //getter function for motor RPM
	return Motor_RPM;
}

float Motor_Velocity_Get(){ //getter function for motor velocity
	return Motor_Velocity;
}
