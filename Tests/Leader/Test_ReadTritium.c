

#include "os.h"
#include "common.h"
#include "Tasks.h"

void Task1(void *);

void main(void) {
    static OS_TCB Task1TCB;
    static CPU_STK Task1Stk[128];

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

    OSQCreate(&CANBus_MsgQ, "CANBus Q", 16, &err);
    if (err != OS_ERR_NONE) {
        printf("OS error code %d\n", err);
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
        (OS_TCB*)&ReadTritium_TCB,
        (CPU_CHAR*)"ReadTritium",
        (OS_TASK_PTR)Task_ReadTritium,
        (void*)NULL,
        (OS_PRIO)2,
        (CPU_STK*)ReadTritium_Stk,
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

    printf("Created ReadTritium\n");

    CANbuff *buf;
    int counter = 0;
    int size;

    while (1) {
        MotorController_Drive(0, counter++);
        printf("sent data\n");
        OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &err);
        buf = OSQPend(&CANBus_MsgQ, 0, OS_OPT_PEND_BLOCKING, &size, &ts, &err);
        printf("Received firstNum: %d\n", buf->firstNum);
    }
}