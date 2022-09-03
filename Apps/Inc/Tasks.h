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
#define TASK_INIT_PRIO                      2
#define TASK_READ_TRITIUM_PRIO              3
#define TASK_UPDATE_VELOCITY_PRIO           4
#define TASK_READ_CAR_CAN_PRIO              5
#define TASK_SEND_DISPLAY_PRIO              6
#define TASK_READ_SWITCHES_PRIO             7
#define TASK_SEND_CAR_CAN_PRIO              8
#define TASK_BLINK_LIGHT_PRIO               9

/**
 * Stack Sizes
 */
#define DEFAULT_STACK_SIZE                  256
#define WATERMARK_STACK_LIMIT               DEFAULT_STACK_SIZE/2

#define TASK_FAULT_STATE_STACK_SIZE         DEFAULT_STACK_SIZE
#define TASK_INIT_STACK_SIZE                DEFAULT_STACK_SIZE
#define TASK_UPDATE_VELOCITY_STACK_SIZE     DEFAULT_STACK_SIZE
#define TASK_READ_CAR_CAN_STACK_SIZE        DEFAULT_STACK_SIZE
#define TASK_SEND_DISPLAY_STACK_SIZE        DEFAULT_STACK_SIZE
#define TASK_READ_TRITIUM_STACK_SIZE        DEFAULT_STACK_SIZE
#define TASK_READ_SWITCHES_STACK_SIZE       DEFAULT_STACK_SIZE
#define TASK_SEND_CAR_CAN_STACK_SIZE        DEFAULT_STACK_SIZE
#define TASK_BLINK_LIGHT_STACK_SIZE         DEFAULT_STACK_SIZE


/**
 * Task Prototypes
 */
void Task_FaultState(void* p_arg);

void Task_Init(void* p_arg);

void Task_UpdateVelocity(void* p_arg);

void Task_ReadCarCAN(void* p_arg);

void Task_SendDisplay(void* p_arg);

void Task_ReadTritium(void* p_arg);

void Task_ReadSwitches(void* p_arg);

void Task_SendCarCAN(void* p_arg);

void Task_BlinkLight(void* p_arg);

/**
 * TCBs
 */
extern OS_TCB FaultState_TCB;
extern OS_TCB Init_TCB;
extern OS_TCB UpdateVelocity_TCB;
extern OS_TCB ReadCarCAN_TCB;
extern OS_TCB SendDisplay_TCB;
extern OS_TCB ReadTritium_TCB;
extern OS_TCB ReadSwitches_TCB;
extern OS_TCB SendCarCAN_TCB;
extern OS_TCB BlinkLight_TCB;


/**
 * Stacks
 */
extern CPU_STK FaultState_Stk[TASK_FAULT_STATE_STACK_SIZE];
extern CPU_STK Init_Stk[TASK_INIT_STACK_SIZE];
extern CPU_STK UpdateVelocity_Stk[TASK_UPDATE_VELOCITY_STACK_SIZE];
extern CPU_STK ReadCarCAN_Stk[TASK_READ_CAR_CAN_STACK_SIZE];
extern CPU_STK SendDisplay_Stk[TASK_SEND_DISPLAY_STACK_SIZE];
extern CPU_STK ReadTritium_Stk[TASK_READ_TRITIUM_STACK_SIZE];
extern CPU_STK ReadSwitches_Stk[TASK_READ_SWITCHES_STACK_SIZE];
extern CPU_STK SendCarCAN_Stk[TASK_SEND_CAR_CAN_STACK_SIZE];
extern CPU_STK BlinkLight_Stk[TASK_BLINK_LIGHT_STACK_SIZE];

/**
 * Queues
 */
extern OS_Q CANBus_MsgQ;

/**
 * Semaphores
 */
extern OS_SEM FaultState_Sem4;


/**
 * Global Variables
 */


//Put all global state variables here
extern bool UpdateVel_ToggleCruise;
extern State RegenEnable;
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
    OS_BLINK_LIGHTS_LOC = 0x040,
    OS_CONTACTOR_LOC = 0x080,
    OS_SWITCHES_LOC = 0x100,
    OS_MAIN_LOC = 0x200,
    OS_CANDRIVER_LOC = 0x400,
    OS_MOTOR_CONNECTION_LOC = 0x800,
    OS_DISPLAY_LOC = 0x1000
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
} fault_bitmap_t;

/**
 * Error variables
 */
extern fault_bitmap_t FaultBitmap;
extern os_error_loc_t OSErrLocBitmap;

/**
 * @brief   Assert Error if OS function call fails
 * @param   OS_err_loc Where OS error occured (driver level)
 * @param   err OS Error that occurred
 */
void _assertOSError(uint16_t OS_err_loc, OS_ERR err); //TODO: This should be changed to enforce only enum usage

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
