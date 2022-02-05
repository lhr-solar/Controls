/* Copyright (c) 2020 UT Longhorn Racing Solar */
#include "os.h"
#include "Tasks.h"
#include "CANbus.h"
#include "BSP_UART.h"
#include "config.h"

/*
 * Note: do not call this directly if it can be helped.
 * Instead, call an RTOS function to unblock the mutex
 * that the Fault Task is pending on.
 */
void EnterFaultState() {
    //switch()
}

void Task_FaultState(void *p_arg) {
    (void)p_arg;
    OS_ERR err;
    CPU_TS ts;

    // BLOCKING =====================
    // Wait until a FAULT is signaled by another task.
    OSSemPend(&Fault_Sem4, 0, OS_OPT_PEND_BLOCKING, &ts, &err);
    
    EnterFaultState();
}