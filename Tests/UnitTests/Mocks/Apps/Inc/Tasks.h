/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#ifdef TEST_TASKS
#include_next "Tasks.h"
#else
#ifndef __TASKS_H
#define __TASKS_H
#include "fff.h"
#include "os.h"
#include "common.h"

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

#ifndef TEST_MAIN
DECLARE_FAKE_VOID_FUNC(Task_Init, void *);
#else
void Task_Init(void* p_arg);
#endif

#ifndef TEST_DEBUGDUMP
DECLARE_FAKE_VOID_FUNC(Task_DebugDump, void*);
#else
void Task_DebugDump(void *p_arg);
#endif

#ifndef TEST_COMMANDLINE
DECLARE_FAKE_VOID_FUNC(Task_CommandLine, void*);
#else
void Task_CommandLine(void* p_arg);
#endif

DECLARE_FAKE_VOID_FUNC(TaskSwHook_Init);

DECLARE_FAKE_VOID_FUNC(EmergencyContactorOpen);

DECLARE_FAKE_VOID_FUNC(throwTaskError, error_code_t, callback_t, error_scheduler_lock_opt_t, error_recov_opt_t);

DECLARE_FAKE_VOID_FUNC(assertOSError, OS_ERR);

#endif
#endif