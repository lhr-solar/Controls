/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file Tasks.c
 * @brief 
 * 
 */

#include "Tasks.h"
#include "os.h"
#include "CANbus.h"
#include "Contactors.h"
#include "Display.h"
#include "Minions.h"
#include "Pedals.h"
#include "ReadTritium.h"
#include "ReadCarCAN.h"
#include "UpdateDisplay.h"


/**
 * TCBs
 */
OS_TCB Init_TCB;
OS_TCB SendTritium_TCB;
OS_TCB ReadCarCAN_TCB;
OS_TCB UpdateDisplay_TCB;
OS_TCB ReadTritium_TCB;
OS_TCB SendCarCAN_TCB;
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
CPU_STK DebugDump_Stk[TASK_DEBUG_DUMP_STACK_SIZE];
CPU_STK CommandLine_Stk[TASK_COMMAND_LINE_STACK_SIZE];

// Variables to store error codes, stored and cleared in task error assert functions
error_code_t Error_ReadCarCAN = /*READCARCAN_ERR_NONE*/ 0; // TODO: change this back to the error 
error_code_t Error_ReadTritium = T_NONE;  // Initialized to no error
error_code_t Error_UpdateDisplay = UPDATEDISPLAY_ERR_NONE;

extern const pinInfo_t PININFO_LUT[]; // For GPIO writes. Externed from Minions Driver C file.

/**
 * Error assertion-related functions
*/

void _assertOSError(OS_ERR err)
{
    if (err != OS_ERR_NONE)
    {
        EmergencyContactorOpen(); // Turn off contactors and turn on the brakelight to indicate an emergency
        Display_Error(err); // Display the location and error code
        while(1){;} //nonrecoverable
    }
}

/**
 * @brief Assert a task error by locking the scheduler (if necessary), displaying a fault screen,
 * and jumping to the error's specified callback function. 
 * Called by task-specific error-assertion functions that are also responsible for setting the error variable.
 * @param errorCode the enum for the specific error that happened
 * @param errorCallback a callback function to a handler for that specific error, 
 * @param lockSched whether or not to lock the scheduler to ensure the error is handled immediately. Only applicable for recoverable errors- nonrecoverable errors will always lock
 * @param nonrecoverable whether or not to kill the motor, display the fault screen, and enter an infinite while loop
*/
void throwTaskError(error_code_t errorCode, callback_t errorCallback, error_scheduler_lock_opt_t lockSched, error_recov_opt_t nonrecoverable) {
    OS_ERR err;

    if (errorCode == 0) { // Exit if there is no error
        return;
    }

    if (lockSched == OPT_LOCK_SCHED || nonrecoverable == OPT_NONRECOV) { // Prevent other tasks from interrupting the handling of important (includes all nonrecoverable) errors
        OSSchedLock(&err);
        assertOSError(err);
    }

    if (nonrecoverable == OPT_NONRECOV) {
        EmergencyContactorOpen();
        Display_Error(errorCode); // Needs to happen before callback so that tasks can change the screen
        // (ex: readCarCAN and evac screen for BPS trip)
    }


    if (errorCallback != NULL) {
        errorCallback(); // Run a handler for this error that was specified in another task file
    }
    

    if (nonrecoverable == OPT_NONRECOV) { // Enter an infinite while loop
        while(1) {

            #if DEBUG == 1
               // Print the error that caused this fault
                // printf("\n\rCurrent Error Code: 0x%04x\n\r", errorCode);

                // // Print the errors for each applications with error data
                // printf("\n\rAll application errors:\n\r");
                // printf("Error_ReadCarCAN: 0x%04x\n\r", Error_ReadCarCAN);
                // printf("Error_ReadTritium: 0x%04x\n\r", Error_ReadTritium);
                // printf("Error_UpdateDisplay: 0x%04x\n\r", Error_UpdateDisplay);

                // // Delay so that we're not constantly printing
                // for (int i = 0; i < 9999999; i++) {
                // } 
            #endif
            
        }
    }

    if (lockSched == OPT_LOCK_SCHED) { // Only happens on recoverable errors
        OSSchedUnlock(&err); 
        // Don't err out if scheduler is still locked because of a timer callback
        if (err != OS_ERR_SCHED_LOCKED || OSSchedLockNestingCtr > 1) { // But we don't plan to lock more than one level deep
           assertOSError(err); 
        }
        
    }
}

/**
 * @brief For use in error handling: opens array and motor precharge bypass contactor
 * and turns on additional brakelight to signal that a critical error happened.
*/
void EmergencyContactorOpen() {
    // Array motor kill
    BSP_GPIO_Write_Pin(CONTACTORS_PORT, MOTOR_CONTROLLER_PRECHARGE_BYPASS_PIN, OFF);
    BSP_GPIO_Write_Pin(CONTACTORS_PORT, ARRAY_PRECHARGE_BYPASS_PIN, OFF);

    // Turn additional brakelight on to indicate critical error
    BSP_GPIO_Write_Pin(PININFO_LUT[BRAKELIGHT].port, PININFO_LUT[BRAKELIGHT].pinMask, true);
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
