/**
 * Copyright (c) 2022 UT Longhorn Racing Solar
 * 
 * Test for the Read Switches task
 * 
 * 
 * * Test Plan * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *    This test prints out the contactor/precharger states every 100ms.
 * 
 * 1. Leave the ignition switch at the "electronics on" position.
 *    Verify that the contactors are correct. The readout should be:
 * 
 *          array precharge | array contactor | motor contactor
 *                OFF       |       OFF       |       OFF
 * 
 * 2. Turn the ignition switch to the "array on" position.
 *    For approximately 2 seconds, the readout should be:
 * 
 *          array precharge | array contactor | motor contactor
 *                ON!       |       OFF       |       OFF
 * 
 *    Then, the readout will switch to (and remain in):
 * 
 *          array precharge | array contactor | motor contactor
 *                OFF       |       ON!       |       OFF
 * 
 * 3. Leave the ignition switch at the "motor on" position.
 *    The readout should be:
 * 
 *          array precharge | array contactor | motor contactor
 *                OFF       |       ON!       |       ON!
 * 
 * 4. Try the following ignition switch sequences and verify that 
 *    the contactors match the readouts listed:
 *      * electronics -> motor
 *      * motor -> array
 *      * array/motor -> electronics
 *      * electronics -> array -> electronics (switch to electronics immediately)
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

    // Initialized with error
    if (err != OS_ERR_NONE) {
        printf("OS error code %d\n", err);
    }

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

    OSStart(&err);
    if (err != OS_ERR_NONE) {
        printf("OS error code %d\n", err);
    }
}

void TestReadSwitches(void *p_arg) {
    OS_ERR err;

    CPU_Init();
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);

    OSTaskCreate(
        (OS_TCB*)&ReadTritium_TCB,
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

    printf("Created ReadSwitches\n");
    // Wait for Task_ReadSwitches to finish initialization
    OSTimeDlyHMSM(0, 0, PRECHARGE_MOTOR_DELAY, 0, OS_OPT_TIME_HMSM_NON_STRICT, &err);

    uint8_t counter = 0;
    while (1) {
        if (counter == 0) {
            printf("array precharge | array contactor | motor contactor\n");
        }
        printf("      %s       |       %s       |       %s      \n",
            (Contactors_Get(ARRAY_PRECHARGE) == ON) ? "ON!" : "OFF",
            (Contactors_Get(ARRAY_CONTACTOR) == ON) ? "ON!" : "OFF",
            (Contactors_Get(MOTOR_CONTACTOR) == ON) ? "ON!" : "OFF");

        counter = (counter + 1) % 50;
        OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_NON_STRICT, &err);
    }
}