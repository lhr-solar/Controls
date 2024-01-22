/**
 * @file Tasks.h
 * @brief 
 * 
 */

#ifndef TASKS_H
#define TASKS_H

#include "common.h"
#include "config.h"
#include "os.h"

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
#define TASK_INIT_PRIO 2
#define TASK_READ_TRITIUM_PRIO 3
#define TASK_SEND_TRITIUM_PRIO 4
#define TASK_READ_CAR_CAN_PRIO 5
#define TASK_SEND_CAR_CAN_PRIO 6
#define TASK_PUT_IOSTATE_PRIO 7
#define TASK_UPDATE_DISPLAY_PRIO 8
#define TASK_DEBUG_DUMP_PRIO 9
#define TASK_COMMAND_LINE_PRIO 10

/**
 * Stack Sizes
 */
#define DEFAULT_STACK_SIZE 256
#define WATERMARK_STACK_LIMIT (DEFAULT_STACK_SIZE / 2)

#define TASK_INIT_STACK_SIZE DEFAULT_STACK_SIZE
#define TASK_SEND_TRITIUM_STACK_SIZE DEFAULT_STACK_SIZE
#define TASK_READ_CAR_CAN_STACK_SIZE DEFAULT_STACK_SIZE
#define TASK_UPDATE_DISPLAY_STACK_SIZE DEFAULT_STACK_SIZE
#define TASK_READ_TRITIUM_STACK_SIZE DEFAULT_STACK_SIZE
#define TASK_SEND_CAR_CAN_STACK_SIZE DEFAULT_STACK_SIZE
#define TASK_DEBUG_DUMP_STACK_SIZE DEFAULT_STACK_SIZE
#define TASK_COMMAND_LINE_STACK_SIZE DEFAULT_STACK_SIZE

/**
 * Task error variable type
 */
typedef uint16_t ErrorCode;

/**
 * Task Prototypes
 */
void TaskInit(void* p_arg);
void TaskSendTritium(void* p_arg);
void TaskReadCarCan(void* p_arg);
void TaskUpdateDisplay(void* p_arg);
void TaskReadTritium(void* p_arg);
void TaskSendCarCan(void* p_arg);
void TaskDebugDump(void* p_arg);
void TaskCommandLine(void* p_arg);

/**
 * TCBs
 */
extern OS_TCB init_tcb;
extern OS_TCB send_tritium_tcb;
extern OS_TCB read_car_can_tcb;
extern OS_TCB update_display_tcb;
extern OS_TCB read_tritium_tcb;
extern OS_TCB send_car_can_tcb;
extern OS_TCB debug_dump_tcb;
extern OS_TCB command_line_tcb;

/**
 * Stacks
 */
extern CPU_STK init_stk[TASK_INIT_STACK_SIZE];
extern CPU_STK send_tritium_stk[TASK_SEND_TRITIUM_STACK_SIZE];
extern CPU_STK read_car_can_stk[TASK_READ_CAR_CAN_STACK_SIZE];
extern CPU_STK update_display_stk[TASK_UPDATE_DISPLAY_STACK_SIZE];
extern CPU_STK read_tritium_stk[TASK_READ_TRITIUM_STACK_SIZE];
extern CPU_STK send_car_can_stk[TASK_SEND_CAR_CAN_STACK_SIZE];
extern CPU_STK debug_dump_stk[TASK_DEBUG_DUMP_STACK_SIZE];
extern CPU_STK command_line_stk[TASK_COMMAND_LINE_STACK_SIZE];

/**
 * Queues
 */
extern OS_Q can_bus_msg_q;

/**
 * @brief Initialize the task switch hook
 * Registers the hook with the RTOS
 */
void TaskSwHookInit(void);

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
    OS_TCB* tasks[TASK_TRACE_LENGTH];
    uint32_t index;
} TaskTrace;

extern TaskTrace prev_tasks;

// Store error codes that are set in task error assertion functions
extern ErrorCode error_read_tritium;
extern ErrorCode error_read_car_can;
extern ErrorCode error_update_display;

/**
 * Error-handling option enums
 */

// Scheduler lock parameter option for asserting a task error
typedef enum { kOptNoLockSched, kOptLockSched } ErrorSchedulerLockOpt;

// Recoverable/nonrecoverable parameter option for asserting a task error
typedef enum { kOptRecov, kOptNonrecov } ErrorRecovOpt;

/**
 * @brief For use in error handling: opens array and motor precharge bypass
 * contactor and turns on additional brakelight to signal that a critical error
 * happened.
 */
void EmergencyContactorOpen();

/**
 * @brief Assert a task error by setting the location variable and optionally
 * locking the scheduler, displaying a fault screen (if nonrecoverable), jumping
 * to a callback function, and entering an infinite loop. Called by
 * task-specific error-assertion functions that are also responsible for setting
 * the error variable.
 * @param errorCode the enum for the specific error that happened
 * @param errorCallback a callback function to a handler for that specific
 * error,
 * @param lockSched whether or not to lock the scheduler to ensure the error is
 * handled immediately
 * @param nonrecoverable whether or not to kill the motor, display the fault
 * screen, and enter an infinite while loop
 */
void ThrowTaskError(ErrorCode error_code, Callback error_callback,
                    ErrorSchedulerLockOpt lock_sched,
                    ErrorRecovOpt nonrecoverable);

/**
 * @brief   Assert Error if OS function call fails
 * @param   err OS Error that occurred
 */
void AssertOsError(OS_ERR err);

#if DEBUG == 1
#define ASSERT_OS_ERROR(err)                                                \
    if (err != OS_ERR_NONE) {                                               \
        printf("Error asserted at %s, line %d: %d\n\r", __FILE__, __LINE__, \
               err);                                                        \
    }                                                                       \
    AssertOsError(err);
#else
#define ASSERT_OS_ERROR(err) AssertOsError(err);
#endif

#endif


