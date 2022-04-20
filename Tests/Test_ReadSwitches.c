/**
 * Copyright (c) 2022 UT Longhorn Racing Solar
 * 
 * Test for the Read Switches task
 * 
 * 
 * * Test Plan * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *    This test prints out the contactor/precharger states every 100ms.
 *    It repeatedly attempts to turn the contactors in order to test the
 *    ReadSwitches thread's ability to enable/disable contactors.
 * 
 * 1. Leave the ignition switch at the "electronics on" position.
 *    Verify that the contactors are correct. The readout should be:
 * 
 *          array precharge | array contactor | motor contactor |  IGN_1  | IGN_2   
 *                OFF       |       OFF       |       OFF       |   OFF   | OFF
 * 
 * 2. Turn the ignition switch to the "array on" position.
 *    ReadSwitches will now enable the array precharge and array 
 *    contactor, and they should come on. 
 * 
 *          array precharge | array contactor | motor contactor |  IGN_1  | IGN_2
 *                ON        |       ON        |       OFF       |   ON    | OFF
 
 * 3. Move the ignition switch at the "motor on" position.
 *    The readout should be:
 * 
 *          array precharge | array contactor | motor contactor |  IGN_1  | IGN_2
 *                ON        |       ON       |       ON         |   ON    | ON
 * 4. Try the following sequences:
 * Electronics -> Array
 * Electronics -> Motor
 * Electronics -> Array -> Electronics
 * Electronics -> Motor -> Electronics
 * Electronics -> Array -> Motor -> Array -> Motor
 */

#include "os.h"
#include "common.h"
#include "Tasks.h"
#include "stm32f4xx.h"
#include "ReadSwitches.h"


void TestReadSwitches(void *);

int main(void) {
    static OS_TCB TestReadSwitchesTCB;
    static CPU_STK TestReadSwitchesStk[DEFAULT_STACK_SIZE];

    OS_ERR err;
    OSInit(&err);

    // FaultState semaphore
    OSSemCreate(&FaultState_Sem4, "FaultState", 0, &err);
    if (err != OS_ERR_NONE) {
        printf("Task error code %d\n", err);
    }
    assertOSError(OS_NONE_LOC, err);

    OSTaskCreate(
        (OS_TCB*)&TestReadSwitchesTCB,
        (CPU_CHAR*)"Task 1",
        (OS_TASK_PTR)TestReadSwitches,
        (void*)NULL,
        (OS_PRIO)1,
        (CPU_STK*)TestReadSwitchesStk,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE/10,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)0,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );

    // Task not created
    if (err != OS_ERR_NONE) {
        printf("Task error code %d\n", err);
    }
    assertOSError(OS_NONE_LOC, err);

    // Start OS
    OSStart(&err);
    if (err != OS_ERR_NONE) {
        printf("OS error code %d\n", err);
    }
    assertOSError(OS_NONE_LOC, err);
}

void TestReadSwitches(void *p_arg) {
    OS_ERR err;

    CPU_Init();
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);

    OSTaskCreate(
        (OS_TCB*)&ReadSwitches_TCB,
        (CPU_CHAR*)"ReadSwitches",
        (OS_TASK_PTR)Task_ReadSwitches,
        (void*)NULL,
        (OS_PRIO)2,
        (CPU_STK*)ReadSwitches_Stk,
        (CPU_STK_SIZE)TASK_READ_SWITCHES_STACK_SIZE/10,
        (CPU_STK_SIZE)TASK_READ_SWITCHES_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)0,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );

    // Task not created
    if (err != OS_ERR_NONE) {
        printf("ReadSwitches task creation error code %d\n", err);
    }
    assertOSError(OS_NONE_LOC, err);

    printf("Created ReadSwitches\n");
    // Wait for Task_ReadSwitches to finish initialization (with an extra second to be sure)
    OSTimeDlyHMSM(0, 0, PRECHARGE_MOTOR_DELAY + 1, 0, OS_OPT_TIME_HMSM_NON_STRICT, &err);

    printf("array precharge | array contactor | motor contactor | IGN1 | IGN2 \n");
    while (1) {
        //attempt to turn on all contactors
        Contactors_Set(ARRAY_PRECHARGE, ON);
        Contactors_Set(ARRAY_CONTACTOR, ON);
        Contactors_Set(MOTOR_CONTACTOR, ON);

        printf(
            "      %s       |       %s       |       %s      |       %s       |       %s       |\r",
            (Contactors_Get(ARRAY_PRECHARGE) == ON) ? "ON!" : "OFF",
            (Contactors_Get(ARRAY_CONTACTOR) == ON) ? "ON!" : "OFF",
            (Contactors_Get(MOTOR_CONTACTOR) == ON) ? "ON!" : "OFF",
            (Switches_Read(IGN_1) == ON) ? "ON!" : "OFF" ,
            (Switches_Read(IGN_2) == ON) ? "ON!" : "OFF" 
        );

        OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_NON_STRICT, &err);
    }
}