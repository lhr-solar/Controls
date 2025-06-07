/* Copyright (c) 2025 UT Longhorn Racing Solar */
#include "Tasks.h"
#include "StatusLeds.h"

#define FLAG_TESTER_PRIO 13
#define FLAG_SETTER_PRIO 14
static OS_TCB Task1TCB;
static CPU_STK Task1Stk[DEFAULT_STACK_SIZE];
static OS_TCB Task2TCB;
static CPU_STK Task2Stk[DEFAULT_STACK_SIZE];

// Flag setter thread
void Task2() {
    OS_ERR err;
    OSTimeDlyHMSM(0, 0, 3, 0, OS_OPT_TIME_HMSM_STRICT, &err);
    assertOSError(err);

    // NOTE: ARRAY_PRECHARGE_CONTACTOR_LED used to show progress on this thread

    // Shouldn't have any effect since we initially pend on BPS_SAFE
    OSFlagPost(&BPS_Motor_Status_Flags, BPS_CHECKED, OS_OPT_POST_FLAG_SET, &err);
    assertOSError(err);
    Status_Leds_Write(ARRAY_PRECHARGE_CONTACTOR_LED, true);
    OSTimeDlyHMSM(0, 0, 3, 0, OS_OPT_TIME_HMSM_STRICT, &err);
    assertOSError(err);

    // Should cause pending on BPS_SAFE to go through
    OSFlagPost(&BPS_Motor_Status_Flags, BPS_CHECKED, OS_OPT_POST_FLAG_SET, &err);
    assertOSError(err);
    Status_Leds_Write(ARRAY_PRECHARGE_CONTACTOR_LED, false);
    OSTimeDlyHMSM(0, 0, 3, 0, OS_OPT_TIME_HMSM_STRICT, &err);
    assertOSError(err);

    // Should cause pending on on all flags to go through
    OSFlagPost(&BPS_Motor_Status_Flags, BPS_SAFE | MOTOR_SAFE_TO_RUN, OS_OPT_POST_FLAG_SET, &err);
    assertOSError(err);
    Status_Leds_Write(ARRAY_PRECHARGE_CONTACTOR_LED, true);
    assertOSError(err);
}

// Tester/pender thread
void Task1() {
    OS_ERR err;
    CPU_TS ticks;
    CPU_Init();
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);
    BPSMotorFlags_Init();
    Status_Leds_Init();

    // create flag setter thread
    OSTaskCreate(
        (OS_TCB *)&Task2TCB,
        (CPU_CHAR *)"Task 2",
        (OS_TASK_PTR)Task2,
        (void *)NULL,
        (OS_PRIO)FLAG_SETTER_PRIO,
        (CPU_STK *)Task2Stk,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE / 10,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void *)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR *)&err);

    // Shouldn't pend here => initializes to all flags clear
    OSFlagPend(&BPS_Motor_Status_Flags, BPS_CHECKED | BPS_SAFE | MOTOR_SAFE_TO_RUN, 0, OS_OPT_PEND_FLAG_CLR_ALL | OS_OPT_PEND_BLOCKING, &ticks, &err);
    assertOSError(err);
    Status_Leds_Write(CONTROLS_FAULT_LED, true);

    OSFlagPend(&BPS_Motor_Status_Flags, BPS_SAFE, 0, OS_OPT_PEND_FLAG_SET_ALL | OS_OPT_PEND_BLOCKING, &ticks, &err);
    assertOSError(err);
    Status_Leds_Write(BPS_FAULT_LED, true);

    OSFlagPend(&BPS_Motor_Status_Flags, BPS_SAFE, 0, OS_OPT_PEND_FLAG_SET_ALL | OS_OPT_PEND_BLOCKING, &ticks, &err);
    assertOSError(err);
    Status_Leds_Write(CRUISE_IND_LED, true);
    // Success!!
    volatile bool pp = true;
    while(pp) {}
}

int main()
{
    OS_ERR err;
    OSInit(&err);

    // create tester thread
    OSTaskCreate(
        (OS_TCB *)&Task1TCB,
        (CPU_CHAR *)"Task 1",
        (OS_TASK_PTR)Task1,
        (void *)NULL,
        (OS_PRIO)FLAG_TESTER_PRIO,
        (CPU_STK *)Task1Stk,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE / 10,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void *)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR *)&err);
    assertOSError(err);

    OSStart(&err);
}