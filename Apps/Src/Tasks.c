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
OS_TCB DebugDump_TCB;
OS_TCB CommandLine_TCB;

task_trace_t PrevTasks;

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
CPU_STK DebugDump_Stk[TASK_DEBUG_DUMP_STACK_SIZE];
CPU_STK CommandLine_Stk[TASK_COMMAND_LINE_STACK_SIZE];

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

/**
 * @brief Hook that's called every context switch
 * 
 * This function will append the task being switched out to the task trace if and only if:
 *      1. It's not a task created automatically by the RTOS
 *      2. It's not the previously recorded task (a long running task interrupted by the
 *         tick task will only show up once)
 * This function will overwrite tasks that have been in the trace for a while, keeping only
 * the 8 most recent tasks
 */
void App_OS_TaskSwHook(void) {
    OS_TCB *cur = OSTCBCurPtr;
    uint32_t idx = PrevTasks.index;
    if (cur == &OSTickTaskTCB) return; // Ignore the tick task
    if (cur == &OSIdleTaskTCB) return; // Ignore the idle task
    if (cur == &OSTmrTaskTCB ) return; // Ignore the timer task
    if (cur == &OSStatTaskTCB) return; // Ignore the stat task
    if (cur == PrevTasks.tasks[idx]) return; // Don't record the same task again
    if (++idx == TASK_TRACE_LENGTH) idx = 0;
    PrevTasks.tasks[idx] = cur;
    PrevTasks.index = idx;
}

void TaskSwHook_Init(void) {
    PrevTasks.index = TASK_TRACE_LENGTH - 1; // List starts out empty
    OS_AppTaskSwHookPtr = App_OS_TaskSwHook;
}