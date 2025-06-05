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

/**
 * Task initialization macro
 * @param task name of the task
 * @param prio the task's priority
 * @param arg the argument to pass to the task
 * @param err the local OS_ERR variable
 */

#define TASK_PROFILER

#ifdef TASK_PROFILER
#define IDLE_PIN           PA15
#define IO_STATE_PIN       PB7
#define READ_CARCAN_PIN    PC14
#define UPDATE_DISPLAY_PIN PA8
#define SEND_CARCAN_PIN    PC12
#endif

/**
 * Priority Definitions
 */

#define TASK_INIT_PRIO                 2
#define TASK_READ_TRITIUM_PRIO         3
#define TASK_SEND_TRITIUM_PRIO         4
#define TASK_READ_CAR_CAN_PRIO         5
#define TASK_SEND_CAR_CAN_PRIO         6
#define TASK_PUT_IOSTATE_PRIO          7
#define TASK_UPDATE_DISPLAY_PRIO       8
#define TASK_DEBUG_DUMP_PRIO           9
#define TASK_COMMAND_LINE_PRIO         10

/**
 * Stack Sizes
 */

#define DEFAULT_STACK_SIZE             256
#define WATERMARK_STACK_LIMIT          DEFAULT_STACK_SIZE / 2

#define TASK_INIT_STACK_SIZE           DEFAULT_STACK_SIZE
#define TASK_SEND_TRITIUM_STACK_SIZE   DEFAULT_STACK_SIZE
#define TASK_READ_CAR_CAN_STACK_SIZE   DEFAULT_STACK_SIZE
#define TASK_UPDATE_DISPLAY_STACK_SIZE DEFAULT_STACK_SIZE
#define TASK_READ_TRITIUM_STACK_SIZE   DEFAULT_STACK_SIZE
#define TASK_SEND_CAR_CAN_STACK_SIZE   DEFAULT_STACK_SIZE
#define TASK_DEBUG_DUMP_STACK_SIZE     DEFAULT_STACK_SIZE
#define TASK_COMMAND_LINE_STACK_SIZE   DEFAULT_STACK_SIZE
#define TASK_IO_STATE_STACK_SIZE       DEFAULT_STACK_SIZE

/**
 * Controls wide error enum.
 */
typedef enum {
    C_ERR_NONE = 0,
    // Read Tritium Errors
    C_ERR_RTR_GENERIC,
    C_ERR_RTR_HARDWARE_OC,
    C_ERR_RTR_SOFTWARE_OC,
    C_ERR_RTR_DC_BUS_OV,
    C_ERR_RTR_HALL_SENSOR,
    C_ERR_RTR_WDOG_LAST_RESET,
    C_ERR_RTR_CONFIG_READ,
    C_ERR_RTR_UNDERVOLT_LOCKOUT,
    C_ERR_RTR_DESAT_FAULT,
    C_ERR_RTR_MOTOR_OVERSPEED,
    C_ERR_RTR_INIT_FAIL,
    C_ERR_RTR_MOTOR_WDOG_TRIP,
    C_ERR_RTR_MULTIPLE,
    C_ERR_RTR_UNKNOWN_ERROR,
    // Send Tritium Errors
    C_ERR_STR_GENERIC,
    C_ERR_STR_GEAR_FAULT,           /* Received multiple or no gear inputs */
    // Read Car CAN Errors
    C_ERR_RCC_GENERIC,
    C_ERR_RCC_BPS_MISSED_MSG,       /* Didn't receive a BPS msg in time (watchdog trip) */
    C_ERR_RCC_PRECHARGE_MISSED_MSG, /* Didn't receive a precharge msg in time (watchdog trip) */
    C_ERR_RCC_BPS_TRIP,             /* Recieved a BPS trip msg */
    C_ERR_RCC_ACTIVE_PRECHARGE_FLT, /* Received active precharge fault */
    C_ERR_RCC_PRECHARGE_TMOUT_MOT,
    C_ERR_RCC_PRECHARGE_TMOUT_ARR,
    // IO state Errors
    C_ERR_IOS_GENERIC,
    C_ERR_IOS_IGN_FAULT,
    // Update display errors
    C_ERR_UPD_GENERIC,
    C_ERR_UPD_PARSE_COMPONENT,
    C_ERR_UPD_DRIVER,
    // Special
    C_ERR_GENERIC,
    C_ERR_ILLEGAL_ERROR,            /* An error is thrown that doesn't belong to the task */

    NUM_CONTROLS_ERRORS
} controls_error_e;

// Max buffer size for the error message string (dictated by the Nextion)
#define ERRMSG_MAX_LEN 16

extern const char ERROR_MSGS[NUM_CONTROLS_ERRORS][ERRMSG_MAX_LEN];

/**
 * BPS & Motor Status Event Flag Definitions
 */

#define BPS_SAFE            1 << 0
#define BPS_CHECKED         1 << 1
#define MOTOR_SAFE_TO_RUN   1 << 2

#define OS_FLAG_BLOCKING    true
#define OS_FLAG_SCHED_POINT true

// Synchronization-protected event flag group signaling BPS_SAFE, if BPS
// has been checked, & motor ready to run status
extern OS_FLAG_GRP BPS_Motor_Status_Flags;

OS_ERR MotorStatus_Wait(uint8_t bits, bool blocking);
OS_FLAGS MotorStatus_GetBits();
bool MotorStatus_ModifyBits(uint8_t bits, bool state, bool allow_sched);

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
// #define ERR_MSG_OFFSET 9
// static inline void set_errmsg_hex(const char *prefix, char *arr, error_code_t err) {
//     arr[0] = '\"';
//     memcpy(arr + 1, prefix, ERR_MSG_OFFSET);
//     snprintf(arr + ERR_MSG_OFFSET, 6, "_%04X", err & 0xFFFF);
//     arr[ERR_MSG_OFFSET + 5] = '\"';
//     arr[ERR_MSG_OFFSET + 6] = '\0';
// }
// #undef ERR_MSG_OFFSET

/**
 * @brief Can be used by tasks to add a delay in ms
 */
inline void delay_ms(uint32_t ms) {
    // Adjusted loop count per ms based on empirical timing
    // Originally: 20,000 per ms (80,000 cycles / 4 cycles/iter)
    // Observed: ~3.77× slower → need ~5300 iterations per ms
    uint32_t count = ms * 5300;

    __asm__ volatile(
        "1: \n"       
        "subs %[cnt], %[cnt], #1 \n"
        "bne 1b \n"
        : [cnt] "+r"(count)
        :
        : "cc"
    );
}

/**
 * Task Prototypes
 */
void Task_Init(void *p_arg);

void Task_StatusLED_Init(void);

void Task_SendTritium(void *p_arg);

void Task_ReadCarCAN(void *p_arg);

void Task_UpdateDisplay(void *p_arg);

void Task_ReadTritium(void *p_arg);

void Task_SendCarCAN(void *p_arg);

void Task_DebugDump(void *p_arg);

void Task_CommandLine(void *p_arg);

void Task_IOState(void *p_arg);

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

/**
 * Scheduler lock parameter option for asserting a task error
 */
typedef enum { OPT_NO_LOCK_SCHED, OPT_LOCK_SCHED } error_scheduler_opt_e;

/**
 * Recoverable/nonrecoverable parameter option for asserting a task error
 */
typedef enum { OPT_RECOV, OPT_NONRECOV } error_recovery_opt_e;

/**
 * @brief Assert a task error by setting the location variable and optionally
 * locking the scheduler, displaying a fault screen (if nonrecoverable), jumping
 * to a callback function, and entering an infinite loop. Called by
 * task-specific error-assertion functions that are also responsible for setting
 * the error variable.
 * @param error_code the enum for the specific error that happened
 * @param is_evac_needed whether evac is required, it will be recommended regardless.
 * @param error_callback a callback function to a handler for that specific
 * error (NULL is permissible),
 * @param lock_scheduler whether or not to lock the scheduler to ensure the
 * error is handled immediately
 * @param recovery whether or not to kill the motor, display the fault
 * screen, and enter an infinite while loop
 */
void throwTaskError(controls_error_e error_code, bool is_evac_needed, callback_t error_callback,
                    error_scheduler_opt_e lock_scheduler, error_recovery_opt_e recovery);

/**
 * @brief   Assert Error if OS function call fails
 * @param   err OS Error that occurred
 */
void _assertOSError(OS_ERR err);

#if DEBUG == 1
#define assertOSError(err)                                                                         \
    if (err != OS_ERR_NONE) {                                                                      \
        printf("Error asserted at %s, line %d: %d\n\r", __FILE__, __LINE__, err);                  \
    }                                                                                              \
    _assertOSError(err);
#else
#define assertOSError(err) _assertOSError(err);
#endif

#endif

/* @} */