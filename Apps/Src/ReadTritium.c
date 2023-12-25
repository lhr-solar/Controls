/**
 * @file ReadTritium.c
 * @details
 * ReadTritium forwards all messages from MotorCAN to CarCAN. It does this using the FIFO 
 * defined in [SendCarCAN](./SendCarCAN.html). The task posts messages to the queue, which are then 
 * read out by the SendCarCAN task.
 * 
 * ReadTritium also facilitates reading velocity and error information in from the motor controller. 
 * Velocity is put on the display, and error information is used to determine if the motor controller 
 * is in a fault state. If the motor controller is in a fault state, the task will attempt to reset the 
 * motor controller. If the motor controller cannot be reset, the task will set the car to a fault state.
 * 
 */
#include "ReadTritium.h"
#include "CANbus.h"
#include "UpdateDisplay.h"
#include "SendCarCAN.h"
#include "os_cfg_app.h"
#include <string.h>

//status limit flag masks
#define MASK_MOTOR_TEMP_LIMIT (1<<6) //check if motor temperature is limiting the motor
#define MAX_CAN_LEN 8
#define RESTART_THRESHOLD 3	// Number of times to restart before asserting a nonrecoverable error
#define MOTOR_TIMEOUT_SECS 1 // Timeout for several missed motor messages
#define MOTOR_TIMEOUT_TICKS (MOTOR_TIMEOUT_SECS * OS_CFG_TMR_TASK_RATE_HZ)

tritium_error_code_t Motor_FaultBitmap = T_NONE; //initialized to no error, changed when the motor asserts an error
static float Motor_RPM = 0;
static float Motor_Velocity = 0;

static OS_TMR MotorWatchdog;

// Function prototypes
static void assertTritiumError(tritium_error_code_t motor_err);

/**
 * @brief Callback function for the motor watchdog timer
 * @param tmr Pointer to the timer
 * @param p_arg unused
*/
static void motorWatchdog(void *tmr, void *p_arg) {
    // Attempt to restart 3 times, then fail
    assertTritiumError(T_MOTOR_WATCHDOG_TRIP);
}

void Task_ReadTritium(void *p_arg){
	OS_ERR err;
	CANDATA_t dataBuf = {0};

	// Timer doesn't seem to trigger without initial delay? Might be an RTOS bug
	OSTmrCreate(&MotorWatchdog, "Motor watchdog", MOTOR_TIMEOUT_TICKS, MOTOR_TIMEOUT_TICKS, OS_OPT_TMR_PERIODIC, motorWatchdog, NULL, &err);
	assertOSError(err);
	OSTmrStart(&MotorWatchdog, &err);
	assertOSError(err);

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
                    OSTmrStart(&MotorWatchdog, &err); // Reset the watchdog
                    assertOSError(err);
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
            
            SendCarCAN_Put(dataBuf); // Forward message on CarCAN for telemetry
		}

		OSTimeDlyHMSM(0, 0, 0, 10, OS_OPT_TIME_HMSM_NON_STRICT, &err);
		assertOSError(err);
	}
}

/**
 * @brief Restarts the motor controller by sending a reset message
*/
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
	static uint8_t motor_fault_cnt = 0;

	Error_ReadTritium = (error_code_t)motor_err; // Store error codes for inspection info
	if(motor_err == T_NONE) return; // No error, return
                                                                                    // NOTE: If we had >1 recoverable errors,
    // Hall sensor error is the only recoverable error, so any other error          // make sure a combination of them wouldn't 
    // or combination of errors includes at least one that is nonrecoverable        // accidentally fall into this nonrecoverable bucket
	if(motor_err != T_HALL_SENSOR_ERR && motor_err != T_MOTOR_WATCHDOG_TRIP){                                             
		// Assert a nonrecoverable error with no callback function- nonrecoverable will kill the motor and infinite loop
		throwTaskError(Error_ReadTritium, NULL, OPT_LOCK_SCHED, OPT_NONRECOV);
		return;
	}

	// If it's purely a hall sensor error, try to restart the motor a few times and then fail out

	if(motor_err == T_HALL_SENSOR_ERR && ++hall_fault_cnt > RESTART_THRESHOLD){  // Threshold has been exceeded
		// Assert a nonrecoverable error that will kill the motor, display a fault screen, and infinite loop
		throwTaskError(Error_ReadTritium, NULL, OPT_LOCK_SCHED, OPT_NONRECOV);
		return;
	}

	//try to restart the motor a few times and then fail out
	if(motor_err == T_MOTOR_WATCHDOG_TRIP && ++motor_fault_cnt > RESTART_THRESHOLD){  
		// Assert a nonrecoverable error that will kill the motor, display a fault screen, and infinite loop
		throwTaskError(Error_ReadTritium, NULL, OPT_LOCK_SCHED, OPT_NONRECOV);
		return;
	}

	// Threshold hasn't been exceeded, so assert a recoverable error with the motor restart callback function
	throwTaskError(Error_ReadTritium, handler_ReadTritium_HallError, OPT_NO_LOCK_SCHED, OPT_RECOV); 
	
	Error_ReadTritium = T_NONE; // Clear the error after handling it
}
