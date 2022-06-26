/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "Tasks.h"
#include "FaultState.h"
#include "os.h"
#include "CANbus.h"
#include "Contactors.h"
#include "Display.h"
#include "Minions.h"
#include "MotorController.h"
#include "Pedals.h"

/**
 * TCBs
 */
OS_TCB FaultState_TCB;
OS_TCB Init_TCB;
OS_TCB UpdateVelocity_TCB;
OS_TCB ReadCarCAN_TCB;
OS_TCB SendDisplay_TCB;
OS_TCB ReadTritium_TCB;
OS_TCB ReadSwitches_TCB;
OS_TCB SendCarCAN_TCB;
OS_TCB BlinkLight_TCB;

/**
 * Stacks
 */
CPU_STK FaultState_Stk[TASK_FAULT_STATE_STACK_SIZE];
CPU_STK Init_Stk[TASK_INIT_STACK_SIZE];
CPU_STK UpdateVelocity_Stk[TASK_UPDATE_VELOCITY_STACK_SIZE];
CPU_STK ReadCarCAN_Stk[TASK_READ_CAR_CAN_STACK_SIZE];
CPU_STK SendDisplay_Stk[TASK_SEND_DISPLAY_STACK_SIZE];
CPU_STK ReadTritium_Stk[TASK_READ_TRITIUM_STACK_SIZE];
CPU_STK ReadSwitches_Stk[TASK_READ_SWITCHES_STACK_SIZE];
CPU_STK SendCarCAN_Stk[TASK_SEND_CAR_CAN_STACK_SIZE];
CPU_STK BlinkLight_Stk[TASK_BLINK_LIGHT_STACK_SIZE];

/**
 * Semaphores
 */
OS_SEM FaultState_Sem4;
OS_SEM VelocityChange_Sem4;
OS_SEM DisplayChange_Sem4;
OS_SEM CarCAN_Sem4;
OS_SEM ReadTritium_Sem4;
OS_SEM BlinkLight_Sem4;
OS_SEM SendCarCAN_Sem4;
OS_SEM ArrayConnectionChange_Sem4;
OS_SEM FaultState_Sem4;

/**
 * Global Variables
 */
// TODO: Put all global state variables here
State RegenEnable = OFF; //whether or not regen is allowed
uint16_t SupplementalVoltage = 0; //supplemental battery pack voltage

fault_bitmap_t FaultBitmap = FAULT_NONE;
os_error_loc_t OSErrLocBitmap = OS_NONE_LOC;

void _assertOSError(uint16_t OS_err_loc, OS_ERR err)
{
    if (err != OS_ERR_NONE)
    {
        FaultBitmap |= FAULT_OS;
        OSErrLocBitmap |= OS_err_loc;

        OSSemPost(&FaultState_Sem4, OS_OPT_POST_1, &err);
        EnterFaultState();
    }
}