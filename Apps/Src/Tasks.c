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
OS_TCB DebugDump_TCB;
OS_TCB CommandLine_TCB;

task_trace_t PrevTasks;

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
CPU_STK DebugDump_Stk[TASK_DEBUG_DUMP_STACK_SIZE];
CPU_STK CommandLine_Stk[TASK_COMMAND_LINE_STACK_SIZE];

/**
 * Global Variables
 */
os_error_loc_t OSErrLocBitmap = OS_NONE_LOC; // Store the location of the current error

// Variables to store error codes, stored and cleared in task error assert functions
error_code_t Error_ReadCarCAN = /*READCARCAN_ERR_NONE*/ 0; // TODO: change this back to the error 
error_code_t Error_ReadTritium = T_NONE;  // Initialized to no error
error_code_t Error_UpdateDisplay = UPDATEDISPLAY_ERR_NONE;

extern const PinInfo_t PINS_LOOKARR[]; // For GPIO writes. Externed from Minions Driver C file.

/**
 * Error assertion-related functions
*/

void _assertOSError(os_error_loc_t OS_err_loc, OS_ERR err)
{
    if (err != OS_ERR_NONE)
    {
        EmergencyContactorOpen(); // Turn off contactors and turn on the brakelight to indicate an emergency
        Display_Error(OS_err_loc, err); // Display the location and error code
        while(1){;} //nonrecoverable

    }
}

/**
 * @brief Assert a task error by locking the scheduler (if necessary), displaying a fault screen,
 * and jumping to the error's specified callback function. 
 * Called by task-specific error-assertion functions that are also responsible for setting the error variable.
 * @param errorLoc the task from which the error originated. Note: should be taken out when last task pointer is integrated
 * @param errorCode the enum for the specific error that happened
 * @param errorCallback a callback function to a handler for that specific error, 
 * @param lockSched whether or not to lock the scheduler to ensure the error is handled immediately. Only applicable for recoverable errors- nonrecoverable errors will always lock
 * @param nonrecoverable whether or not to kill the motor, display the fault screen, and enter an infinite while loop
*/
void assertTaskError(os_error_loc_t errorLoc, error_code_t errorCode, callback_t errorCallback, error_scheduler_lock_opt_t lockSched, error_recov_opt_t nonrecoverable) {
    OS_ERR err;

    if (lockSched == OPT_LOCK_SCHED || nonrecoverable == OPT_NONRECOV) { // Prevent other tasks from interrupting the handling of important (includes all nonrecoverable) errors
        OSSchedLock(&err);
        assertOSError(OS_TASKS_LOC, err);
    }

    // Set the location error variable
    OSErrLocBitmap = errorLoc; 

    if (nonrecoverable == OPT_NONRECOV) {
        EmergencyContactorOpen(); // Apart from while loop because killing the motor is more important
        Display_Error(errorLoc, errorCode); // Needs to happen before callback so that tasks can change the screen
        // (ex: readCarCAN and evac screen for BPS trip)
    }


    if (errorCallback != NULL) {
        errorCallback(); // Run a handler for this error that was specified in another task file
    }
    

    if (nonrecoverable == OPT_NONRECOV) { // Enter an infinite while loop
        while(1) {

            // Print the error that caused this fault
            printf("\n\rCurrent Error Location: 0x%04x", OSErrLocBitmap);
            printf("\n\rCurrent Error Code: 0x%04x\n\r", errorCode);

            // Print the errors for each applications with error data
            printf("\n\rAll application errors:\n\r");
            printf("Error_ReadCarCAN: 0x%04x\n\r", Error_ReadCarCAN);
            printf("Error_ReadTritium: 0x%04x\n\r", Error_ReadTritium);
            printf("Error_UpdateDisplay: 0x%04x\n\r", Error_UpdateDisplay);

            // Delay so that we're not constantly printing
            for (int i = 0; i < 9999999; i++) {
            }
        }
    }

    if (lockSched == OPT_LOCK_SCHED) { // Only happens on recoverable errors
        OSSchedUnlock(&err); 
        // Don't err out if scheduler is still locked because of a timer callback
        if (err != OS_ERR_SCHED_LOCKED && OSSchedLockNestingCtr > 1) { // But we don't plan to lock more than one level deep
           assertOSError(OS_TASKS_LOC, err); 
        }
        
    }

    OSErrLocBitmap = OS_NONE_LOC; // Clear the location error variable once handled
}

/**
 * @brief For use in error handling: opens array and motor precharge bypass contactor
 * and turns on additional brakelight to signal that a critical error happened.
*/
void EmergencyContactorOpen() {
    // Array motor kill
    BSP_GPIO_Write_Pin(CONTACTORS_PORT, MOTOR_CONTACTOR_PIN, OFF);
    BSP_GPIO_Write_Pin(CONTACTORS_PORT, ARRAY_PRECHARGE_PIN, OFF);

    // Turn additional brakelight on to indicate critical error
    BSP_GPIO_Write_Pin(PINS_LOOKARR[BRAKELIGHT].port, PINS_LOOKARR[BRAKELIGHT].pinMask, true);
}

/**
 * @brief Hook that's called every context switch
 * 
 * This function will append the task being switched out to the task trace if and only if:
 *      1. It's not a task created automatically by the RTOS
 *      2. It's not the previously recorded task (a long running task interrupted by the
 *         tick task will only show up once)
 * This function will overwrite tasks that have been in the trace for a while, keeping only
 * the 8 most recent tasks
 */
void App_OS_TaskSwHook(void) {
    OS_TCB *cur = OSTCBCurPtr;
    uint32_t idx = PrevTasks.index;
    if (cur == &OSTickTaskTCB) return; // Ignore the tick task
    if (cur == &OSIdleTaskTCB) return; // Ignore the idle task
    if (cur == &OSTmrTaskTCB ) return; // Ignore the timer task
    if (cur == &OSStatTaskTCB) return; // Ignore the stat task
    if (cur == PrevTasks.tasks[idx]) return; // Don't record the same task again
    if (++idx == TASK_TRACE_LENGTH) idx = 0;
    PrevTasks.tasks[idx] = cur;
    PrevTasks.index = idx;
}

void TaskSwHook_Init(void) {
    PrevTasks.index = TASK_TRACE_LENGTH - 1; // List starts out empty
    OS_AppTaskSwHookPtr = App_OS_TaskSwHook;
}
