/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "common.h"
#include "config.h"
#include <bsp.h>
#include <unistd.h>
#include "CANbus.h"
#include "stm32f4xx.h"
#include <os.h>

// Globals
static OS_TCB Task1TCB;
static CPU_STK Task1Stk[128];

// Task prototypes
void Task1(void *p_arg);

void main(void)
{
    OS_ERR err;
    OSInit(&err);

    // Initialized with error
    if (err != OS_ERR_NONE)
    {
        printf("OS error code %d\n", err);
    }

    OSTaskCreate(
        (OS_TCB *)&Task1TCB,
        (CPU_CHAR *)"Task 1",
        (OS_TASK_PTR)Task1,
        (void *)NULL,
        (OS_PRIO)1,
        (CPU_STK *)Task1Stk,
        (CPU_STK_SIZE)128 / 10,
        (CPU_STK_SIZE)128,
        (OS_MSG_QTY)NULL,
        (OS_TICK)NULL,
        (void *)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR *)&err);

    // Task not created
    if (err != OS_ERR_NONE)
    {
        printf("Task error code %d\n", err);
    }

    OSStart(&err);
    if (err != OS_ERR_NONE)
    {
        printf("OS error code %d\n", err);
    }
}

void Task1(void *p_arg)
{
    CPU_Init();
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U)OSCfg_TickRate_Hz);

    // Initialize CAN 1, the car can
    CANbus_Init();

    // Receiving buffer that will be populated
    uint8_t buffer[8];

    while (1)
    {
        // Attempt to read from CANbus_Read
        if (CANbus_Read(1, buffer, CAN_BLOCKING) == SUCCESS)
        {
            // Delay to be able to read buffer in GDB
            for (int i = 0; i < 100000; i++)
            {
            }
        }
    }
}
