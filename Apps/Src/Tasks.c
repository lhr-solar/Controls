/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "Tasks.h"
#include "FaultState.h"
#include "os.h"
#include "CANbus.h"
#include "Contactors.h"
#include "Display.h"
#include "Minions.h"
#include "Pedals.h"

/**
 * TCBs
 */
OS_TCB FaultState_TCB;
OS_TCB Init_TCB;
OS_TCB SendTritium_TCB;
OS_TCB ReadCarCAN_TCB;
OS_TCB UpdateDisplay_TCB;
OS_TCB ReadTritium_TCB;
OS_TCB SendCarCAN_TCB;
OS_TCB Telemetry_TCB;

task_trace_t PrevTasks;

/**
 * Stacks
 */
CPU_STK FaultState_Stk[TASK_FAULT_STATE_STACK_SIZE];
CPU_STK Init_Stk[TASK_INIT_STACK_SIZE];
CPU_STK SendTritium_Stk[TASK_SEND_TRITIUM_STACK_SIZE];
CPU_STK ReadCarCAN_Stk[TASK_READ_CAR_CAN_STACK_SIZE];
CPU_STK UpdateDisplay_Stk[TASK_UPDATE_DISPLAY_STACK_SIZE];
CPU_STK ReadTritium_Stk[TASK_READ_TRITIUM_STACK_SIZE];
CPU_STK SendCarCAN_Stk[TASK_SEND_CAR_CAN_STACK_SIZE];
CPU_STK Telemetry_Stk[TASK_TELEMETRY_STACK_SIZE];


/**
 * Semaphores
 */
OS_SEM FaultState_Sem4;

/**
 * Global Variables
 */
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

void App_OS_TaskSwHook(void) {
    OS_TCB *cur = OSTCBCurPtr;
    if (cur == &OSTickTaskTCB) return; // Ignore the tick task, since it happens a lot
    if (cur == PrevTasks.tasks[PrevTasks.index]) return; // Don't record the same task again
    PrevTasks.index = (PrevTasks.index + 1) & 7;
    PrevTasks.tasks[PrevTasks.index] = cur;
}

void TaskSwHook_Init(void) {
    PrevTasks.index = 0xffffffff; // List starts out empty
    OS_AppTaskSwHookPtr = App_OS_TaskSwHook;
}