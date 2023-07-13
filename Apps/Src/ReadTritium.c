/* Copyright (c) 2021 UT Longhorn Racing Solar */

#include "ReadTritium.h"
#include "CAN_Queue.h"
#include "CANbus.h"
#include "UpdateDisplay.h"
#include <string.h>

//status limit flag masks
#define MASK_MOTOR_TEMP_LIMIT (1<<6) //check if motor temperature is limiting the motor 6
#define MAX_CAN_LEN 8
#define RESTART_THRESHOLD 3	// Number of times to restart before asserting a nonrecoverable error


tritium_error_code_t Motor_FaultBitmap = T_NONE; //initialized to no error, changed when the motor asserts an error
static float Motor_RPM = MOTOR_STOPPED; //initialized to 0, motor would be "stopped" until a motor velocity is read
static float Motor_Velocity = CAR_STOPPED; //initialized to 0, car would be "stopped" until a car velocity is read

// Function prototypes
static void assertTritiumError(tritium_error_code_t motor_err);

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


/**
 * Error handler functions
 * Passed as callback functions to the main assertTaskError function by assertTritiumError
*/

/**
 * @brief A callback function to be run by the main assertTaskError function for hall sensor errors
 * restart the motor if the number of hall errors is still less than the RESTART_THRESHOLD.
 */ 
static void handler_Tritium_HallError(void) {
	MotorController_Restart(); 
}


/**
 * @brief   Assert a Tritium error by checking Motor_FaultBitmap
 * and asserting the error with its handler callback if one exists.
 *  Can result in restarting the motor (while < RESTART_THRESHOLD number of hall sensor errors)
 * or locking the scheduler and entering a nonrecoverable fault (all other cases)
 * @param   motor_err Bitmap with motor error codes to check
 */
static void assertTritiumError(tritium_error_code_t motor_err){   
	static uint8_t hall_fault_cnt = 0; //trip counter, doesn't ever reset

	Error_ReadTritium = motor_err; // Store error code for inspection info
	if(motor_err == T_NONE) return; // No error, return
	
	if(motor_err != T_HALL_SENSOR_ERR){
		// Assert a nonrecoverable error with no callback function- nonrecoverable will kill the motor and infinite loop
		assertTaskError(OS_READ_TRITIUM_LOC, motor_err, NULL, OPT_LOCK_SCHED, OPT_NONRECOV);
		return;
	}

	hall_fault_cnt++; 

	//try to restart the motor a few times and then fail out
	if(hall_fault_cnt > RESTART_THRESHOLD){  
		// Assert a nonrecoverable error that will kill the motor, display a fault screen, and infinite loop
		assertTaskError(OS_READ_TRITIUM_LOC, motor_err, NULL, OPT_LOCK_SCHED, OPT_NONRECOV);
		return;
	} 
	// Try restarting the motor
	// Assert a recoverable error that will run the motor restart callback function
	assertTaskError(OS_READ_TRITIUM_LOC, motor_err, handler_Tritium_HallError, OPT_NO_LOCK_SCHED, OPT_RECOV); 
	
	Error_ReadTritium = T_NONE; // Clear the error after handling it
}