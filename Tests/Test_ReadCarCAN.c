#include "common.h"
#include "config.h"
#include "os.h"
#include "Tasks.h"

void Task1(void *);

void main(void) {
    static OS_TCB Task1_TCB;
    static CPU_STK Task1_STK[128];

    OS_ERR err;
    OSInit(&err);

    // Initialized with error
    if (err != OS_ERR_NONE) {
        printf("OS error code %d\n", err);
    }

    OSTaskCreate(
        (OS_TCB*)&Task1_TCB,
        (CPU_CHAR*)"Task 1",
        (OS_TASK_PTR)Task1,
        (void*)NULL,
        (OS_PRIO)3,
        (CPU_STK*)Task1_STK,
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

void Task1(void *p_arg) {

    OS_ERR err;
    CPU_TS ts;

    CPU_Init();
    OS_CPU_SysTickInit();

    OSTaskCreate(
        (OS_TCB*)&ReadCarCAN_TCB,
        (CPU_CHAR*)"ReadCarCAN",
        (OS_TASK_PTR)Task_ReadCarCAN,
        (void*)NULL,
        (OS_PRIO)4,
        (CPU_STK*)ReadCarCAN_Stk,
        (CPU_STK_SIZE)128/10,
        (CPU_STK_SIZE)128,
        (OS_MSG_QTY)NULL,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );

    printf("Spawned ReadCarCAN\n");

    OSTimeDlyHMSM(0, 0, 3, 0, OS_OPT_TIME_HMSM_STRICT, &err);
    printf("Delayed for 3 seconds\n");
    OSTaskSemPost(&ReadCarCAN_TCB, OS_OPT_POST_NONE, &err);
    OSTimeDlyHMSM(0, 0, 3, 0, OS_OPT_TIME_HMSM_STRICT, &err);
    printf("Delayed for 3 seconds\n");
    OSTaskSemPost(&ReadCarCAN_TCB, OS_OPT_POST_NONE, &err);
}