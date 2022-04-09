/* Copyright (c) 2020 UT Longhorn Racing Solar */

#ifndef __TASKS_H
#define __TASKS_H

#include "common.h"
#include "os.h"
#include "common.h"
#include "CarState.h"

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
#define TASK_SEND_TRITIUM_PRIO              2
#define TASK_UPDATE_VELOCITY_PRIO           3
#define TASK_READ_CAR_CAN_PRIO              4
#define TASK_SEND_DISPLAY_PRIO              5
#define TASK_READ_PEDALS_PRIO               6
#define TASK_READ_TRITIUM_PRIO              7
#define TASK_READ_SWITCHES_PRIO             8
#define TASK_UPDATE_LIGHTS_PRIO             9
#define TASK_SEND_CAR_CAN_PRIO              10
#define TASK_BLINK_LIGHT_PRIO               11
#define TASK_ARRAY_CONNECTION_PRIO          12
#define TASK_MOTOR_CONNECTION_PRIO          13
#define TASK_IDLE_PRIO                      14

/**
 * Stack Sizes
 */
#define DEFAULT_STACK_SIZE                  256
#define WATERMARK_STACK_LIMIT               DEFAULT_STACK_SIZE/2

#define TASK_FAULT_STATE_STACK_SIZE         DEFAULT_STACK_SIZE
#define TASK_SEND_TRITIUM_STACK_SIZE        DEFAULT_STACK_SIZE
#define TASK_UPDATE_VELOCITY_STACK_SIZE     DEFAULT_STACK_SIZE
#define TASK_READ_CAR_CAN_STACK_SIZE        DEFAULT_STACK_SIZE
#define TASK_SEND_DISPLAY_STACK_SIZE        DEFAULT_STACK_SIZE
#define TASK_READ_PEDALS_STACK_SIZE         DEFAULT_STACK_SIZE
#define TASK_READ_TRITIUM_STACK_SIZE        DEFAULT_STACK_SIZE
#define TASK_READ_SWITCHES_STACK_SIZE       DEFAULT_STACK_SIZE
#define TASK_UPDATE_LIGHTS_STACK_SIZE       DEFAULT_STACK_SIZE
#define TASK_SEND_CAR_CAN_STACK_SIZE        DEFAULT_STACK_SIZE
#define TASK_BLINK_LIGHT_STACK_SIZE         DEFAULT_STACK_SIZE
#define TASK_ARRAY_CONNECTION_STACK_SIZE    DEFAULT_STACK_SIZE
#define TASK_MOTOR_CONNECTION_STACK_SIZE    DEFAULT_STACK_SIZE
#define TASK_IDLE_STACK_SIZE                DEFAULT_STACK_SIZE
#define TASK_INIT_STACK_SIZE                DEFAULT_STACK_SIZE

/**
 * Task Prototypes
 */
void Task_FaultState(void* p_arg);

void Task_SendTritium(void* p_arg); 

void Task_UpdateVelocity(void* p_arg);

void Task_ReadCarCAN(void* p_arg);

void Task_SendDisplay(void* p_arg);

void Task_ReadPedals(void* p_arg);

void Task_ReadTritium(void* p_arg);

void Task_ReadSwitches(void* p_arg);

void Task_UpdateLights(void* p_arg);

void Task_SendCarCAN(void* p_arg);

void Task_BlinkLight(void* p_arg);

void Task_ArrayConnection(void* p_arg);

void Task_MotorConnection(void* p_arg);

void Task_Idle(void* p_arg);

void Task_Init(void* p_arg);

/**
 * TCBs
 */
extern OS_TCB FaultState_TCB;
extern OS_TCB SendTritium_TCB;
extern OS_TCB UpdateVelocity_TCB;
extern OS_TCB ReadCarCAN_TCB;
extern OS_TCB SendDisplay_TCB;
extern OS_TCB ReadPedals_TCB;
extern OS_TCB ReadTritium_TCB;
extern OS_TCB ReadSwitches_TCB;
extern OS_TCB UpdateLights_TCB;
extern OS_TCB SendCarCAN_TCB;
extern OS_TCB BlinkLight_TCB;
extern OS_TCB ArrayConnection_TCB;
extern OS_TCB MotorConnection_TCB;
extern OS_TCB Idle_TCB;
extern OS_TCB Init_TCB;

/**
 * Stacks
 */
extern CPU_STK FaultState_Stk[TASK_FAULT_STATE_STACK_SIZE];
extern CPU_STK SendTritium_Stk[TASK_SEND_TRITIUM_STACK_SIZE];
extern CPU_STK UpdateVelocity_Stk[TASK_UPDATE_VELOCITY_STACK_SIZE];
extern CPU_STK ReadCarCAN_Stk[TASK_READ_CAR_CAN_STACK_SIZE];
extern CPU_STK SendDisplay_Stk[TASK_SEND_DISPLAY_STACK_SIZE];
extern CPU_STK ReadPedals_Stk[TASK_READ_PEDALS_STACK_SIZE];
extern CPU_STK ReadTritium_Stk[TASK_READ_TRITIUM_STACK_SIZE];
extern CPU_STK ReadSwitches_Stk[TASK_READ_SWITCHES_STACK_SIZE];
extern CPU_STK UpdateLights_Stk[TASK_UPDATE_LIGHTS_STACK_SIZE];
extern CPU_STK SendCarCAN_Stk[TASK_SEND_CAR_CAN_STACK_SIZE];
extern CPU_STK BlinkLight_Stk[TASK_BLINK_LIGHT_STACK_SIZE];
extern CPU_STK ArrayConnection_Stk[TASK_ARRAY_CONNECTION_STACK_SIZE];
extern CPU_STK MotorConnection_Stk[TASK_MOTOR_CONNECTION_STACK_SIZE];
extern CPU_STK Idle_Stk[TASK_IDLE_STACK_SIZE];
extern CPU_STK Init_Stk[TASK_INIT_STACK_SIZE];

/**
 * Queues
 */
extern OS_Q CANBus_MsgQ;

/**
 * Semaphores
 */
extern OS_SEM FaultState_Sem4;
extern OS_SEM VelocityChange_Sem4;
extern OS_SEM DisplayChange_Sem4;
extern OS_SEM LightsChange_Sem4;
extern OS_SEM CarCAN_Sem4;
extern OS_SEM SendTritium_Sem4;
extern OS_SEM ReadTritium_Sem4;
extern OS_SEM ActivateArray_Sem4;
extern OS_SEM ActivateMotor_Sem4;
extern OS_SEM BlinkLight_Sem4;
extern OS_SEM SendCarCAN_Sem4;
extern OS_SEM ArrayConnectionChange_Sem4;
extern OS_SEM MotorConnectionChange_Sem4;

/**
 * Global Variables
 */


// TODO: Put all global state variables here

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
} os_error_loc_t;

/**
 * Fault Union
 * 
 * Different fault states that need to be handled by the FaultState task
 */
typedef union{
    uint8_t bitmap;
    struct{
        State Fault_OS : 1;         // for OS faults
        State Fault_UNREACH : 1;    // for unreachable conditions
        State Fault_TRITIUM : 1;      // for errors sent from the tritium
        State Fault_READBPS : 1;    // for unsuccessfully reading from BPS CAN
        State Fault_DISPLAY : 1;    // for display faults
    };
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
void assertOSError(uint16_t OS_err_loc, OS_ERR err);

#endif
