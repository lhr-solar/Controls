/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "Tasks.h"
#include "os.h"
#include "CANbus.h"
#include "Contactors.h"
#include "Display.h"
#include "Minions.h"
#include "Pedals.h"
#include "ReadTritium.h" // For ReadTritium error enum
#include "ReadCarCAN.h" // For ReadCarCAN error enum
#include "UpdateDisplay.h" // For update display error enum


/**
 * TCBs
 */
OS_TCB Init_TCB;
OS_TCB SendTritium_TCB;
OS_TCB ReadCarCAN_TCB;
OS_TCB UpdateDisplay_TCB;
OS_TCB ReadTritium_TCB;
OS_TCB SendCarCAN_TCB;
OS_TCB Telemetry_TCB;

/**
 * Stacks
 */
CPU_STK Init_Stk[TASK_INIT_STACK_SIZE];
CPU_STK SendTritium_Stk[TASK_SEND_TRITIUM_STACK_SIZE];
CPU_STK ReadCarCAN_Stk[TASK_READ_CAR_CAN_STACK_SIZE];
CPU_STK UpdateDisplay_Stk[TASK_UPDATE_DISPLAY_STACK_SIZE];
CPU_STK ReadTritium_Stk[TASK_READ_TRITIUM_STACK_SIZE];
CPU_STK SendCarCAN_Stk[TASK_SEND_CAR_CAN_STACK_SIZE];
CPU_STK Telemetry_Stk[TASK_TELEMETRY_STACK_SIZE];


/**
 * Global Variables
 */
os_error_loc_t OSErrLocBitmap = OS_NONE_LOC;
error_code_t Error_ReadTritium = T_NONE;  // Variables to store error codes, stored and cleared in task error assert functions
//error_code_t Error_ReadCarCAN = RCC_ERR_NONE;
error_code_t Error_UpdateDisplay = UPDATEDISPLAY_ERR_NONE;
extern const PinInfo_t PINS_LOOKARR[]; // For GPIO writes. Externed from Minions Driver C file.


/**
 * Error assertion-related functions
*/

void _assertOSError(uint16_t OS_err_loc, OS_ERR err)
{
    if (err != OS_ERR_NONE)
    {
        printf("\n\rOS Error code %d", err);
        BSP_GPIO_Write_Pin(PINS_LOOKARR[BRAKELIGHT].port, PINS_LOOKARR[BRAKELIGHT].pinMask, true);
        
        BSP_GPIO_Write_Pin(CONTACTORS_PORT, ARRAY_CONTACTOR_PIN, OFF);
        BSP_GPIO_Write_Pin(CONTACTORS_PORT, MOTOR_CONTACTOR_PIN, OFF);
        while(1){;} //nonrecoverable

    }
}

/**
 * @brief Assert a task error by locking the scheduler (if necessary), displaying a fault screen,
 * and jumping to the error's specified callback function
 * @param schedLock whether or not to lock the scheduler to ensure the error is handled immediately
 * @param errorLoc the task from which the error originated. TODO: should be taken out when last task pointer is integrated
 * @param faultCode the value for what specific error happened
 * @param errorCallback a callback function to a handler for that specific error
*/
void assertTaskError(os_error_loc_t errorLoc, uint8_t errorCode, callback_t errorCallback, error_lock_sched_opt_t lockSched, error_recov_opt_t nonrecoverable) {
    OS_ERR err;

    if (lockSched == OPT_LOCK_SCHED) { // We want this error to be handled immediately without other tasks being able to interrupt
        OSSchedLock(&err);
        assertOSError(OS_TASKS_LOC, err);
    }

    // Set the error variables to store data //TODO: delete? 
    OSErrLocBitmap = errorLoc; 

    if (nonrecoverable == OPT_NONRECOV) {
        arrayMotorKill(); // Apart from while loop because killing the motor is more important
        Display_Error(errorLoc, errorCode); // Needs to happen before callback so that tasks can change the screen
        // (ex: readCarCAN and evac screen for BPS trip)
    }


    if (errorCallback != NULL) {
        errorCallback(); // Run a handler for this error that was specified in another task file
    }
    

    if (nonrecoverable == OPT_NONRECOV) { // enter an infinite while loop
        while(1){;}
    }

    if (lockSched == OPT_LOCK_SCHED) { // Only happens on recoverable errors
        OSSchedUnlock(&err); 
        assertOSError(OS_TASKS_LOC, err);
    }
}

void arrayMotorKill() {
    // Array motor kill
    BSP_GPIO_Write_Pin(CONTACTORS_PORT, ARRAY_CONTACTOR_PIN, OFF);
    BSP_GPIO_Write_Pin(CONTACTORS_PORT, MOTOR_CONTACTOR_PIN, OFF);

    // Turn additional brakelight on to indicate critical error
    BSP_GPIO_Write_Pin(PINS_LOOKARR[BRAKELIGHT].port, PINS_LOOKARR[BRAKELIGHT].pinMask, true);
}
