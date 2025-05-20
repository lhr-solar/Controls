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
#include "IOState.h"
#include "Pedals.h"
#include "ReadTritium.h"
#include "ReadCarCAN.h"
#include "UpdateDisplay.h"
#include "daybreak_pins.h"
#include "StatusLeds.h"

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
OS_TCB IOState_TCB;

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
CPU_STK IOState_Stk[TASK_IO_STATE_STACK_SIZE];

#define DISP_NA_STR_LITERAL "\"N/A\""
#define DISP_EVAC_NONREQ_STR_LITERAL "\"RECOMMENDED\""
#define DISP_EVAC_REQ_STR_LITERAL "\"REQUIRED!!!\""

// Controls Fault Message bits
#define ANY_CONTROLS_FAULT_BIT 1        // 1 if any of the other bits are true
#define MOTOR_CONTROLLER_FAULT_BIT 2    // 1 if there is a ReadTritium Error
#define BPS_FAULT_BIT 4                 // 1 if there is a BPS Trip error
// #define PEDALS_FAULT_BIT 8           // ""
#define READCARCAN_FAULT_BIT 16         // 1 if there is a ReadCarCAN Error
#define DISPLAY_FAULT_BIT 32            // 1 if there is an UpdateDisplay Error
#define OS_FAULT_BIT 64                     // 1 if there is an OS Error
// #define LAKSHAY_FAULT_BIT 128        // Not sure what this is

#define FAULT_MSG_DELAY 100000

const char *DISP_ERRMSG_NA = DISP_NA_STR_LITERAL;
const char *DISP_EVACMSG_DEFAULT = DISP_EVAC_NONREQ_STR_LITERAL;
const char *DISP_EVACMAG_REQ = DISP_EVAC_REQ_STR_LITERAL;

// Variables to store error codes, stored and cleared in task error assert functions
error_code_t Error_ReadCarCAN = READCARCAN_ERR_NONE; // TODO: change this back to the error 
error_code_t Error_ReadTritium = T_NONE;  // Initialized to no error
error_code_t Error_UpdateDisplay = UPDATEDISPLAY_ERR_NONE;
error_code_t Error_OS = OS_ERR_NONE;

// Display error messages for readability
char ErrMsg_ReadCarCAN[ERR_CODE_LEN] = DISP_NA_STR_LITERAL;
char ErrMsg_ReadTritium[ERR_CODE_LEN] = DISP_NA_STR_LITERAL;
char ErrMsg_UpdateDisplay[ERR_CODE_LEN] = DISP_NA_STR_LITERAL;
char ErrMsg_OS[ERR_CODE_LEN] = DISP_NA_STR_LITERAL;
char ErrMsg_Evac[ERR_CODE_LEN] = DISP_EVAC_NONREQ_STR_LITERAL;


extern const pinInfo_t PININFO_LUT[]; // For GPIO writes. Externed from Minions Driver C file.

/**
 * @brief Check and set error bits for CONTROLS_FAULT_MSG
 * @return a byte with the error bits set according to Controls' current faults
 */
uint8_t get_fault_bits(error_code_t errorCode) {
    
    uint8_t msg = 0;

    if (Error_ReadCarCAN != READCARCAN_ERR_NONE)        {msg |= READCARCAN_FAULT_BIT;}
    if (Error_ReadTritium != T_NONE)                    {msg |= MOTOR_CONTROLLER_FAULT_BIT;}
    if (Error_UpdateDisplay != UPDATEDISPLAY_ERR_NONE)  {msg |= DISPLAY_FAULT_BIT;}
    if (Error_OS != OS_ERR_NONE)                        {msg |= OS_FAULT_BIT;}

    if (errorCode == READCARCAN_ERR_BPS_TRIP)           {msg |= BPS_FAULT_BIT;}

    if (msg != 0)                                       {msg |= ANY_CONTROLS_FAULT_BIT;}

    return msg;
}

/**
 * Error assertion-related functions
 */

void _assertOSError(OS_ERR err)
{
    if (err != OS_ERR_NONE)
    {
        Status_Leds_Write(OS_FAULT_LED, ON);
        Error_OS = err;
        snprintf(ErrMsg_OS, ERR_CODE_LEN, "%08X", Error_OS);
        MotorContactor_EmergencyDisable(); // Turn off all contactors
        Display_Error(); // Display the location and error code

        CANDATA_t faultmsg = {0};
        faultmsg.ID = CONTROLS_FAULT_MSG;
        faultmsg.data[0] = get_fault_bits(NULL); // No errCode - Won't know if it's a BPS trip message
        
        volatile static int faultLoopCount = 0;

        while(1){ //nonrecoverable
            faultLoopCount++;
            if (faultLoopCount > FAULT_MSG_DELAY){
                faultLoopCount = 0;
                CANbus_Send_Faultstate(faultmsg, CARCAN);
            }
        }
            
           
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

    Status_Leds_Write(CONTROLS_FAULT_LED, ON);
    if (errorCode == 0) { // Exit if there is no error
        return;
    }

    if (lockSched == OPT_LOCK_SCHED || nonrecoverable == OPT_NONRECOV) { // Prevent other tasks from interrupting the handling of important (includes all nonrecoverable) errors
        OSSchedLock(&err);
        assertOSError(err);
    }

    if (nonrecoverable == OPT_NONRECOV) {
        MotorContactor_EmergencyDisable();
        Display_Error(); // Needs to happen before callback so that tasks can change the screen
        // (ex: readCarCAN and evac screen for BPS trip)
    }


    if (errorCallback != NULL) {
        errorCallback(); // Run a handler for this error that was specified in another task file
    }

    // Set CAN Message data for Controls Fault
    CANDATA_t faultmsg = {0};
    faultmsg.ID = CONTROLS_FAULT_MSG;

    // Check and set errors
    faultmsg.data[0] = get_fault_bits(errorCode);

    CANbus_Send_Faultstate(faultmsg, CARCAN);


    if (nonrecoverable == OPT_NONRECOV) { // Enter an infinite while loop
        volatile static int faultLoopCount = 0;
        while(1) {
            faultLoopCount++;

            // periodically toggle Controls Fault LED
            if(faultLoopCount > 9999999){
                faultLoopCount = 0;
                Status_Leds_Toggle(CONTROLS_FAULT_LED);
                Status_Leds_Toggle(DASH_HEARTBEAT_LED);
            }
            // periodically resend the Controls Fault message
            if ((faultLoopCount % FAULT_MSG_DELAY) == 0){
                CANbus_Send_Faultstate(faultmsg, CARCAN);
            }
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
    // only reaches here is fault is recoverable

    if (lockSched == OPT_LOCK_SCHED) { // Only happens on recoverable errors
        Status_Leds_Write(CONTROLS_FAULT_LED, OFF);
        OSSchedUnlock(&err); 
        // Don't err out if scheduler is still locked because of a timer callback
        if (err != OS_ERR_SCHED_LOCKED || OSSchedLockNestingCtr > 1) { // But we don't plan to lock more than one level deep
        assertOSError(err); 
        }
    }
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
