/* Copyright (c) 2020 UT Longhorn Racing Solar */

#ifndef __TASKS_H
#define __TASKS_H

#include "common.h"
#include "os.h"
#include "config.h"

/**
 * Task initialization macro
 * @param task name of the task
 * @param prio the task's priority
 * @param arg the argument to pass to the task
 * @param err the local OS_ERR variable
 */

/**
 * Priority Definitions
 */ 
#define TASK_INIT_PRIO                      2
#define TASK_READ_TRITIUM_PRIO              3
#define TASK_SEND_TRITIUM_PRIO              4
#define TASK_READ_CAR_CAN_PRIO              5
#define TASK_UPDATE_DISPLAY_PRIO            6
#define TASK_SEND_CAR_CAN_PRIO              8
#define TASK_TELEMETRY_PRIO                 9
#define TASK_DEBUG_DUMP_PRIO                10
#define TASK_COMMAND_LINE_PRIO              11

/**
 * Stack Sizes
 */
#define DEFAULT_STACK_SIZE                  256
#define WATERMARK_STACK_LIMIT               DEFAULT_STACK_SIZE/2

#define TASK_INIT_STACK_SIZE                DEFAULT_STACK_SIZE
#define TASK_SEND_TRITIUM_STACK_SIZE        DEFAULT_STACK_SIZE
#define TASK_READ_CAR_CAN_STACK_SIZE        DEFAULT_STACK_SIZE
#define TASK_UPDATE_DISPLAY_STACK_SIZE      DEFAULT_STACK_SIZE
#define TASK_READ_TRITIUM_STACK_SIZE        DEFAULT_STACK_SIZE
#define TASK_SEND_CAR_CAN_STACK_SIZE        DEFAULT_STACK_SIZE
#define TASK_TELEMETRY_STACK_SIZE           DEFAULT_STACK_SIZE
#define TASK_DEBUG_DUMP_STACK_SIZE          DEFAULT_STACK_SIZE
#define TASK_COMMAND_LINE_STACK_SIZE        DEFAULT_STACK_SIZE

/**
 * Task error variable type
*/
typedef uint16_t error_code_t;

/**
 * Task Prototypes
 */
void Task_Init(void* p_arg);

void Task_SendTritium(void* p_arg);

void Task_ReadCarCAN(void* p_arg);

void Task_UpdateDisplay(void* p_arg);

void Task_ReadTritium(void* p_arg);

void Task_SendCarCAN(void* p_arg);

void Task_Telemetry(void* p_arg);

void Task_DebugDump(void *p_arg);

void Task_CommandLine(void* p_arg);



/**
 * TCBs
 */
extern OS_TCB Init_TCB;
extern OS_TCB SendTritium_TCB;
extern OS_TCB ReadCarCAN_TCB;
extern OS_TCB UpdateDisplay_TCB;
extern OS_TCB ReadTritium_TCB;
extern OS_TCB SendCarCAN_TCB;
extern OS_TCB Telemetry_TCB;
extern OS_TCB DebugDump_TCB;
extern OS_TCB CommandLine_TCB;


/**
 * Stacks
 */
extern CPU_STK Init_Stk[TASK_INIT_STACK_SIZE];
extern CPU_STK SendTritium_Stk[TASK_SEND_TRITIUM_STACK_SIZE];
extern CPU_STK ReadCarCAN_Stk[TASK_READ_CAR_CAN_STACK_SIZE];
extern CPU_STK UpdateDisplay_Stk[TASK_UPDATE_DISPLAY_STACK_SIZE];
extern CPU_STK ReadTritium_Stk[TASK_READ_TRITIUM_STACK_SIZE];
extern CPU_STK SendCarCAN_Stk[TASK_SEND_CAR_CAN_STACK_SIZE];
extern CPU_STK Telemetry_Stk[TASK_TELEMETRY_STACK_SIZE];
extern CPU_STK DebugDump_Stk[TASK_DEBUG_DUMP_STACK_SIZE];
extern CPU_STK CommandLine_Stk[TASK_COMMAND_LINE_STACK_SIZE];

/**
 * Queues
 */
extern OS_Q CANBus_MsgQ;

/**
 * @brief Initialize the task switch hook
 * Registers the hook with the RTOS
 */
void TaskSwHook_Init(void);


/**
 * Global Variables
 */


/**
 * OS Error States
 * 
 * Stores error data to indicate which part of the code
 * an error is coming from.
 */
typedef enum{
    OS_NONE_LOC = 0x000,
    OS_ARRAY_LOC = 0x001,
    OS_READ_CAN_LOC = 0x002,
    OS_READ_TRITIUM_LOC = 0x004,
    OS_SEND_CAN_LOC = 0x008,
    OS_SEND_TRITIUM_LOC = 0x010,
    OS_UPDATE_VEL_LOC = 0x020,
    OS_CONTACTOR_LOC = 0x080,
    OS_MINIONS_LOC = 0x100,
    OS_MAIN_LOC = 0x200,
    OS_CANDRIVER_LOC = 0x400,
    OS_MOTOR_CONNECTION_LOC = 0x800,
    OS_DISPLAY_LOC = 0x1000,
    OS_TASKS_LOC = 0x2000,
} os_error_loc_t;

/**
 * Task trace
 * 
 * Stores the last TASK_TRACE_LENGTH tasks that were run
 * The most recent task is at tasks[index], the one before at tasks[index-1],
 * wrapping back around at the beginnning
 * 
 */
#define TASK_TRACE_LENGTH 8
typedef struct {
    OS_TCB *tasks[TASK_TRACE_LENGTH];
    uint32_t index;
} task_trace_t;

extern task_trace_t PrevTasks;

/**
 * Error variables
 */
extern os_error_loc_t OSErrLocBitmap;

// Store error codes that are set in task error assertion functions
extern error_code_t Error_ReadTritium; 
extern error_code_t Error_ReadCarCAN;
extern error_code_t Error_UpdateDisplay;

/**
 * Error-handling option enums
*/

// Scheduler lock parameter option for asserting a task error
typedef enum {
    OPT_NO_LOCK_SCHED,
    OPT_LOCK_SCHED
} error_scheduler_lock_opt_t;

// Recoverable/nonrecoverable parameter option for asserting a task error
typedef enum {
    OPT_RECOV,
    OPT_NONRECOV
} error_recov_opt_t;

/**
 * @brief For use in error handling: turns off array, array precharge, and motor contactor
 * and turns on additional brakelight to signal that a critical error happened.
*/
void arrayMotorKill();

/**
 * @brief Assert a task error by setting the location variable and optionally locking the scheduler, 
 * displaying a fault screen (if nonrecoverable), jumping to a callback function, and entering an infinite loop. 
 * Called by task-specific error-assertion functions that are also responsible for setting the error variable.
 * @param errorLoc the task from which the error originated. Note: should be taken out when last task pointer is integrated
 * @param errorCode the enum for the specific error that happened
 * @param errorCallback a callback function to a handler for that specific error, 
 * @param schedLock whether or not to lock the scheduler to ensure the error is handled immediately
 * @param nonrecoverable whether or not to kill the motor, display the fault screen, and enter an infinite while loop
*/
void assertTaskError(os_error_loc_t errorLoc, error_code_t errorCode, callback_t errorCallback, error_scheduler_lock_opt_t lockSched, error_recov_opt_t nonrecoverable);

/**
 * @brief   Assert Error if OS function call fails
 * @param   OS_err_loc Where OS error occured (driver level)
 * @param   err OS Error that occurred
 */
void _assertOSError(os_error_loc_t OS_err_loc, OS_ERR err); 

#if DEBUG == 1
#define assertOSError(OS_err_loc,err) \
        if (err != OS_ERR_NONE) { \
            printf("Error asserted at %s, line %d: %d\n\r", __FILE__, __LINE__, err); \
        } \
        _assertOSError(OS_err_loc,err);
#else
#define assertOSError(OS_err_loc,err) _assertOSError(OS_err_loc,err);
#endif

#endif
