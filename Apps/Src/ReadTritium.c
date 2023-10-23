/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file ReadTritium.c
 * @brief 
 * 
 */

#include "ReadTritium.h"
#include "CAN_Queue.h"
#include "CANbus.h"
#include "UpdateDisplay.h"
#include <string.h>

//status limit flag masks
#define MOTOR_RESTART_THRESHOLD 3	// Number of times to restart before asserting a nonrecoverable error


tritium_error_code_t Motor_FaultBitmap = T_NONE;
static float Motor_RPM = 0;
static float Motor_Velocity = 0;

// Function prototypes
static void assertTritiumError(tritium_error_code_t motor_err);





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

static void restartMotorController(void){
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
 * Passed as callback functions to the main throwTaskError function by assertTritiumError
*/

/**
 * @brief A callback function to be run by the main throwTaskError function for hall sensor errors
 * restart the motor if the number of hall errors is still less than the MOTOR_RESTART_THRESHOLD.
 */ 
static inline void handler_ReadTritium_HallError(void) {
	restartMotorController(); 
}


/**
 * @brief   Assert a Tritium error by checking Motor_FaultBitmap
 * and asserting the error with its handler callback if one exists.
 *  Can result in restarting the motor (for hall sensor errors while less than MOTOR_RESTART_THRESHOLD)
 * or locking the scheduler and entering a nonrecoverable fault (all other cases)
 * @param   motor_err Bitmap with motor error codes to check
 */
static void assertTritiumError(tritium_error_code_t motor_err){   
	static uint8_t hall_fault_cnt = 0; //trip counter, doesn't ever reset

	Error_ReadTritium = (error_code_t)motor_err; // Store error codes for inspection info
	if(motor_err == T_NONE) return; // No error, return
                                                                                    // NOTE: If we had >1 recoverable errors,
    // Hall sensor error is the only recoverable error, so any other error          // make sure a combination of them wouldn't 
    // or combination of errors includes at least one that is nonrecoverable        // accidentally fall into this nonrecoverable bucket
	if(motor_err != T_HALL_SENSOR_ERR){                                             
		// Assert a nonrecoverable error with no callback function- nonrecoverable will kill the motor and infinite loop
		throwTaskError(Error_ReadTritium, NULL, OPT_LOCK_SCHED, OPT_NONRECOV);
		return;
	}

	// If it's purely a hall sensor error, try to restart the motor a few times and then fail out

	if(++hall_fault_cnt > MOTOR_RESTART_THRESHOLD){  // Threshold has been exceeded
		// Assert a nonrecoverable error that will kill the motor, display a fault screen, and infinite loop
		throwTaskError(Error_ReadTritium, NULL, OPT_LOCK_SCHED, OPT_NONRECOV);
		return;
	} 

	// Threshold hasn't been exceeded, so assert a recoverable error with the motor restart callback function
	throwTaskError(Error_ReadTritium, handler_ReadTritium_HallError, OPT_NO_LOCK_SCHED, OPT_RECOV); 
	
	Error_ReadTritium = T_NONE; // Clear the error after handling it
}