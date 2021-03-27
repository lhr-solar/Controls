/* Copyright (c) 2020 UT Longhorn Racing Solar */

#ifndef __TASKS_H
#define __TASKS_H

#include "os.h"
#include "CarState.h"

/**
 * Priority Definitions
 */
#define TASK_SEND_TRITIUM_PRIO              0
#define TASK_UPDATE_VELOCITY_PRIO           1
#define TASK_READ_CAR_CAN_PRIO              2
#define TASK_SEND_DISPLAY_PRIO              3
#define TASK_READ_PEDALS_PRIO               4
#define TASK_READ_TRITIUM_PRIO              5
#define TASK_READ_SWITCHES_PRIO             6
#define TASK_UPDATE_LIGHTS_PRIO             7
#define TASK_SEND_CAR_CAN_PRIO              8
#define TASK_BLINK_LIGHT_PRIO               9
#define TASK_ARRAY_CONNECTION_PRIO          10
#define TASK_MOTOR_CONNECTION_PRIO          11
#define TASK_IDLE_PRIO                      12

/**
 * Stack Sizes
 */
#define DEFAULT_STACK_SIZE                  256
#define WATERMARK_STACK_LIMIT               DEFAULT_STACK_SIZE/2

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

/**
 * Global Variables
 */


// TODO: Put all global state variables here

#endif
