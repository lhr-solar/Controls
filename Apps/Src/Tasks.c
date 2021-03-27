/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "Tasks.h"
#include "os.h"

/**
 * TCBs
 */
OS_TCB SendTritium_TCB;
OS_TCB UpdateVelocity_TCB;
OS_TCB ReadCarCAN_TCB;
OS_TCB SendDisplay_TCB;
OS_TCB ReadPedals_TCB;
OS_TCB ReadTritium_TCB;
OS_TCB ReadSwitches_TCB;
OS_TCB UpdateLights_TCB;
OS_TCB SendCarCAN_TCB;
OS_TCB BlinkLight_TCB;
OS_TCB ArrayConnection_TCB;
OS_TCB MotorConnection_TCB;
OS_TCB Idle_TCB;
OS_TCB Init_TCB;

/**
 * Stacks
 */
CPU_STK SendTritium_Stk[TASK_SEND_TRITIUM_STACK_SIZE];
CPU_STK UpdateVelocity_Stk[TASK_UPDATE_VELOCITY_STACK_SIZE];
CPU_STK ReadCarCAN_Stk[TASK_READ_CAR_CAN_STACK_SIZE];
CPU_STK SendDisplay_Stk[TASK_SEND_DISPLAY_STACK_SIZE];
CPU_STK ReadPedals_Stk[TASK_READ_PEDALS_STACK_SIZE];
CPU_STK ReadTritium_Stk[TASK_READ_TRITIUM_STACK_SIZE];
CPU_STK ReadSwitches_Stk[TASK_READ_SWITCHES_STACK_SIZE];
CPU_STK UpdateLights_Stk[TASK_UPDATE_LIGHTS_STACK_SIZE];
CPU_STK SendCarCAN_Stk[TASK_SEND_CAR_CAN_STACK_SIZE];
CPU_STK BlinkLight_Stk[TASK_BLINK_LIGHT_STACK_SIZE];
CPU_STK ArrayConnection_Stk[TASK_ARRAY_CONNECTION_STACK_SIZE];
CPU_STK MotorConnection_Stk[TASK_MOTOR_CONNECTION_STACK_SIZE];
CPU_STK Idle_Stk[TASK_IDLE_STACK_SIZE];
CPU_STK Init_Stk[TASK_INIT_STACK_SIZE];

/**
 * Queues
 */
OS_Q CANBus_MsgQ;

/**
 * Semaphores
 */
OS_SEM VelocityChange_Sem4;
OS_SEM DisplayChange_Sem4;
OS_SEM LightsChange_Sem4;
OS_SEM CarCAN_Sem4;
OS_SEM SendTritium_Sem4;
OS_SEM ReadTritium_Sem4;
OS_SEM ActivateArray_Sem4;
OS_SEM ActivateMotor_Sem4;
OS_SEM BlinkLight_Sem4;
OS_SEM SendCarCAN_Sem4;

/**
 * Global Variables
 */

// TODO: Put all global state variables here

