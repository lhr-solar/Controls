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
#include "IOState.h"
// #include "config.h"

/**
 * Task initialization macro
 * @param task name of the task
 * @param prio the task's priority
 * @param arg the argument to pass to the task
 * @param err the local OS_ERR variable
 */

#define TASK_PROFILER

#ifdef TASK_PROFILER
#define IDLE_PIN PA15
#define IO_STATE_PIN PB7
#define READ_CARCAN_PIN PC14
#define UPDATE_DISPLAY_PIN PA8
#define SEND_CARCAN_PIN PC12
#endif

/**
 * Priority Definitions
 */ 
#define TASK_INIT_PRIO                      2
#define TASK_READ_TRITIUM_PRIO              3
#define TASK_SEND_TRITIUM_PRIO              4
#define TASK_READ_CAR_CAN_PRIO              5
#define TASK_SEND_CAR_CAN_PRIO              6
#define TASK_PUT_IOSTATE_PRIO               7
#define TASK_UPDATE_DISPLAY_PRIO            8
#define TASK_DEBUG_DUMP_PRIO                9
#define TASK_COMMAND_LINE_PRIO              10

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
#define TASK_IO_STATE_STACK_SIZE            DEFAULT_STACK_SIZE

#define DISP_NA_STR_LITERAL "\"N/A\""
#define DISP_EVAC_NONREQ_STR_LITERAL "\"V('u')V\""
#define DISP_EVAC_REQ_STR_LITERAL "\"REQUIRED!!!\""


/**
 * BPS & Motor Status Event Flag Definitions
 */

#define BPS_SAFE 1 << 0
#define BPS_CHECKED 1 << 1
#define MOTOR_SAFE_TO_RUN 1 << 2

// Synchronization-protected event flag group signaling BPS_SAFE, if BPS
// has been checked, & motor ready to run status
extern OS_FLAG_GRP BPS_Motor_Status_Flags;

/**
 * Task error variable type
 */
typedef uint16_t error_code_t;

/**
 * Macro for adding error id to the error 
 * message array for displaying the fault.
 * Error id is a 16 bit number, so it is
 * split into 4 hex characters.
 */
// uOS_ERR_XXXX
#define ERR_MSG_OFFSET 9
static inline void set_errmsg_hex(const char *prefix, char *arr, error_code_t err) {
    arr[0] = '\"';
    memcpy(arr + 1, prefix, 9);
    snprintf(arr + ERR_MSG_OFFSET, 6, "_%04X", err & 0xFFFF);
    arr[ERR_MSG_OFFSET + 5] = '\"';
    arr[ERR_MSG_OFFSET + 6] = '\0';
}
#undef ERR_MSG_OFFSET

extern const char *DISP_ERRMSG_NA;
extern const char *DISP_EVACMSG_DEFAULT;
extern const char *DISP_EVACMSG_REQ;

/**
 * @brief Can be used by tasks to add a delay in ms
 */
inline void delay_ms(uint32_t ms) {
    // Adjusted loop count per ms based on empirical timing
    // Originally: 20,000 per ms (80,000 cycles / 4 cycles/iter)
    // Observed: ~3.77× slower → need ~5300 iterations per ms
    uint32_t count = ms * 5300;

    __asm__ volatile (
        "1: \n"
        "subs %[cnt], %[cnt], #1 \n"
        "bne 1b \n"
        : [cnt] "+r" (count)
        :
        : "cc"
    );
}

/**
 * Task Prototypes
 */
void Task_Init(void* p_arg);

void Task_StatusLED_Init(void);

void Task_SendTritium(void* p_arg);

void Task_ReadCarCAN(void* p_arg);

void Task_UpdateDisplay(void* p_arg);

void Task_ReadTritium(void* p_arg);

void Task_SendCarCAN(void* p_arg);

void Task_DebugDump(void *p_arg);

void Task_CommandLine(void* p_arg);

void Task_IOState(void* p_arg);

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
extern OS_TCB IOState_TCB;


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
extern CPU_STK IOState_Stk[TASK_IO_STATE_STACK_SIZE];

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
 * @brief Initializes the BPS & Motor
 * Flags for use between tasks
 */
void BPSMotorFlags_Init(void);

/**
 * Task trace
 * 
 * Stores the last TASK_TRACE_LENGTH tasks that were run
 * The most recent task is at tasks[index], the one before at tasks[index-1],
 * wrapping back around at the beginning
 * 
 */
#define TASK_TRACE_LENGTH 8
typedef struct {
    OS_TCB *tasks[TASK_TRACE_LENGTH];
    uint32_t index;
} task_trace_t;

extern task_trace_t PrevTasks;

// Store error codes that are set in task error assertion functions
extern error_code_t Error_SendTritium;
extern error_code_t Error_ReadTritium; 
extern error_code_t Error_ReadCarCAN;
extern error_code_t Error_UpdateDisplay;
extern error_code_t Error_IOState;
extern error_code_t Error_OS;

// Define the length of the error code message
#define ERR_CODE_LEN 16

extern char ErrMsg_SendTritium[ERR_CODE_LEN];
extern char ErrMsg_ReadTritium[ERR_CODE_LEN];
extern char ErrMsg_ReadCarCAN[ERR_CODE_LEN];
extern char ErrMsg_UpdateDisplay[ERR_CODE_LEN];
extern char ErrMsg_IOState[ERR_CODE_LEN];
extern char ErrMsg_OS[ERR_CODE_LEN];
extern char ErrMsg_Evac[ERR_CODE_LEN];

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