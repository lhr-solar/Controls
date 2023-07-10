#include "common.h"
#include "config.h"
#include "stm32f4xx.h"
#include <os.h>

// Globals
static OS_TCB Task1TCB;
static OS_TCB Task2TCB;
static CPU_STK Task1Stk[128];
static CPU_STK Task2Stk[128];

// Task prototypes
void Task1(void* p_arg);
void Task2(void* p_arg);

void main(void) {
    OS_ERR err;
    OSInit(&err);

    // Initialized with error
    if (err != OS_ERR_NONE) {
        printf("OS error code %d\n", err);
    }

    OSTaskCreate(
        (OS_TCB*)&Task1TCB,
        (CPU_CHAR*)"Task 1",
        (OS_TASK_PTR)Task1,
        (void*)NULL,
        (OS_PRIO)1,
        (CPU_STK*)Task1Stk,
        (CPU_STK_SIZE)128/10,
        (CPU_STK_SIZE)128,
        (OS_MSG_QTY)NULL,
        (OS_TICK)NULL,
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

// Task 1
void Task1(void* p_arg) {
    OS_ERR err;

    CPU_Init();
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);

    OSTaskCreate(
        (OS_TCB*)&Task2TCB,
        (CPU_CHAR*)"Task 2",
        (OS_TASK_PTR)Task2,
        (void*)NULL,
        (OS_PRIO)2,
        (CPU_STK*)Task2Stk,
        (CPU_STK_SIZE)0,
        (CPU_STK_SIZE)128,
        (OS_MSG_QTY)NULL,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );

    // Task not created
    if (err != OS_ERR_NONE) {
        printf("Task error code %d\n", err);
    }

    while (1) {
        printf("Task 1\n");
        OSTimeDly(
            (OS_TICK) 1,
            (OS_OPT) OS_OPT_TIME_DLY,
            (OS_ERR*)&err
        );
    }
}

// Task 2
void Task2(void* p_arg) {
    OS_ERR err;
    while (1) {
        printf("Task 2\n");
        OSTimeDly(
            (OS_TICK) 1,
            (OS_OPT) OS_OPT_TIME_DLY,
            (OS_ERR*)&err
        );
    }
}
