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
#define TASK_FAULT_STATE_PRIO               1
#define TASK_FAULT_HIGHPRIO_PRIO            1   
#define TASK_INIT_PRIO                      2
#define TASK_FAULT_LOWPRIO_PRIO             3
#define TASK_READ_TRITIUM_PRIO              4
#define TASK_SEND_TRITIUM_PRIO              5
#define TASK_READ_CAR_CAN_PRIO              6
#define TASK_UPDATE_DISPLAY_PRIO            7
#define TASK_SEND_CAR_CAN_PRIO              8
#define TASK_TELEMETRY_PRIO                 9

/**
 * Stack Sizes
 */
#define DEFAULT_STACK_SIZE                  256
#define WATERMARK_STACK_LIMIT               DEFAULT_STACK_SIZE/2

#define TASK_FAULT_STATE_STACK_SIZE         DEFAULT_STACK_SIZE
#define TASK_INIT_STACK_SIZE                DEFAULT_STACK_SIZE
#define TASK_SEND_TRITIUM_STACK_SIZE        DEFAULT_STACK_SIZE
#define TASK_READ_CAR_CAN_STACK_SIZE        DEFAULT_STACK_SIZE
#define TASK_UPDATE_DISPLAY_STACK_SIZE      DEFAULT_STACK_SIZE
#define TASK_READ_TRITIUM_STACK_SIZE        DEFAULT_STACK_SIZE
#define TASK_SEND_CAR_CAN_STACK_SIZE        DEFAULT_STACK_SIZE
#define TASK_TELEMETRY_STACK_SIZE           DEFAULT_STACK_SIZE

/**
 * Task Prototypes
 */
void Task_FaultState(void* p_arg);

void Task_Init(void* p_arg);

void Task_SendTritium(void* p_arg);

void Task_ReadCarCAN(void* p_arg);

void Task_UpdateDisplay(void* p_arg);

void Task_ReadTritium(void* p_arg);

void Task_SendCarCAN(void* p_arg);

void Task_Telemetry(void* p_arg);







/**
 * TCBs
 */
extern OS_TCB FaultState_TCB; // To be deleted
extern OS_TCB Init_TCB;
extern OS_TCB SendTritium_TCB;
extern OS_TCB ReadCarCAN_TCB;
extern OS_TCB UpdateDisplay_TCB;
extern OS_TCB ReadTritium_TCB;
extern OS_TCB SendCarCAN_TCB;
extern OS_TCB Telemetry_TCB;



/**
 * Stacks
 */
extern CPU_STK FaultState_Stk[TASK_FAULT_STATE_STACK_SIZE]; // To be deleted
extern CPU_STK Init_Stk[TASK_INIT_STACK_SIZE];
extern CPU_STK SendTritium_Stk[TASK_SEND_TRITIUM_STACK_SIZE];
extern CPU_STK ReadCarCAN_Stk[TASK_READ_CAR_CAN_STACK_SIZE];
extern CPU_STK UpdateDisplay_Stk[TASK_UPDATE_DISPLAY_STACK_SIZE];
extern CPU_STK ReadTritium_Stk[TASK_READ_TRITIUM_STACK_SIZE];
extern CPU_STK SendCarCAN_Stk[TASK_SEND_CAR_CAN_STACK_SIZE];
extern CPU_STK Telemetry_Stk[TASK_TELEMETRY_STACK_SIZE];


/**
 * Queues
 */
extern OS_Q CANBus_MsgQ;


/**
 * Global Variables
 */


//Put all global state variables here
extern bool UpdateVel_ToggleCruise;
extern uint16_t SupplementalVoltage;
extern uint32_t StateOfCharge;

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
    OS_FAULT_STATE_LOC = 0x2000, // to be deleted
    OS_TASKS_LOC = 0x2000,
} os_error_loc_t;

/**
 * Fault Enum
 * 
 * Different fault states that need to be handled by the FaultState task
 */
typedef enum{
    FAULT_NONE = 0x00,      // No fault
    FAULT_OS = 0x01,         // for OS faults
    FAULT_UNREACH = 0x02,    // for unreachable conditions
    FAULT_TRITIUM = 0x04,      // for errors sent from the tritium
    FAULT_READBPS = 0x08,    // for unsuccessfully reading from BPS CAN
    FAULT_DISPLAY = 0x10,    // for display faults
    FAULT_BPS = 0x20,       // for if BPS trips
} fault_bitmap_t;

/**
 * Error variables
 */
extern fault_bitmap_t FaultBitmap;
extern os_error_loc_t OSErrLocBitmap;

/**
 * Fault Exception Enum
 * For the "prio" field of a fault exception struct to determine handling mode
 */
typedef enum {
    PRI_RESERVED = 0, // Currently unused, reserved prio of max severity for future use
    PRI_NONRECOV = 1, // Kills contactors, turns on lights, and enters a nonrecoverable fault
    PRI_RECOV = 2, // Returns to thread (no action besides message and callback)

} exception_prio_t;

/**
 * Fault Exception Struct
 * Created by different tasks and passed to fault thread by asserting errors
 * Priority: 1 for nonrecoverable fault, 2 for callback only
 * Callback function will run first regardless of priority
*/
typedef struct{
    exception_prio_t prio;
    const char* message; 
    void (*callback)(void);
} exception_t;

/**
 * @brief For use in error handling: turns off array and motor contactor, turns on additional brakelight
 * to signal a ciritical error happened. Separated from nonrecoverable fault handler 
*/
void arrayMotorKill();

/**
 * @brief Used by error assertion functions to handle and signal a critical error
 * Kills contactors, turns on brakelights, displays the fault screen with error code,
 * and enters an infinite while loop
 * TODO: add paramaters to display fault screen
 * @param osLocCode Location code to display the name of the task, currently uses the OS_ERR_LOC enum
 * @param faultCode The enum value for the task's specific error code
*/
void nonrecoverableErrorHandler(os_error_loc_t osLocCode, uint8_t faultCode);

/**
 * @brief Assert a task error by locking the scheduler (if necessary), displaying a fault screen,
 * and jumping to the error's specified callback function
 * @param errorLoc the task from which the error originated. TODO: should be taken out when last task pointer is integrated
 * @param faultCode the value for what specific error happened
 * @param errorCallback a callback function to a handler for that specific error
 * @param schedLock whether or not to lock the scheduler to ensure the error is handled immediately
 * @param nonrecoverable if true, kills the motor, displays the fault screen, and enters an infinite while loop
*/
void assertTaskError(os_error_loc_t errorLoc, uint8_t faultCode, callback_t errorCallback, bool lockSched, bool nonrecoverable);

/**
 * @brief   Assert Error if OS function call fails
 * @param   OS_err_loc Where OS error occured (driver level)
 * @param   err OS Error that occurred
 */
void _assertOSError(uint16_t OS_err_loc, OS_ERR err); 

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
