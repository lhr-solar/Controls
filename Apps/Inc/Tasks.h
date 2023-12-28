/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file Tasks.h
 * @brief 
 * 
 * @defgroup Tasks
 * @addtogroup Tasks
 * @{
 */


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
#define TASK_SEND_CAR_CAN_PRIO              7
#define TASK_DEBUG_DUMP_PRIO                8
#define TASK_COMMAND_LINE_PRIO              9

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
 * @brief For use in error handling: opens array and motor precharge bypass contactor
 * and turns on additional brakelight to signal that a critical error happened.
*/
void EmergencyContactorOpen();

/**
 * @brief Assert a task error by setting the location variable and optionally locking the scheduler, 
 * displaying a fault screen (if nonrecoverable), jumping to a callback function, and entering an infinite loop. 
 * Called by task-specific error-assertion functions that are also responsible for setting the error variable.
 * @param errorCode the enum for the specific error that happened
 * @param errorCallback a callback function to a handler for that specific error, 
 * @param lockSched whether or not to lock the scheduler to ensure the error is handled immediately
 * @param nonrecoverable whether or not to kill the motor, display the fault screen, and enter an infinite while loop
*/
void throwTaskError(error_code_t errorCode, callback_t errorCallback, error_scheduler_lock_opt_t lockSched, error_recov_opt_t nonrecoverable);

/**
 * @brief   Assert Error if OS function call fails
 * @param   err OS Error that occurred
 */
void _assertOSError (OS_ERR err); //TODO: This should be changed to enforce only enum usage

#if DEBUG == 1
#define assertOSError(err) \
        if (err != OS_ERR_NONE) { \
            printf("Error asserted at %s, line %d: %d\n\r", __FILE__, __LINE__, err); \
        } \
        _assertOSError(err);
#else
#define assertOSError(err) _assertOSError(err);
#endif

#endif

/* @} */