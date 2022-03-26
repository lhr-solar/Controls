

#include "os.h"
#include "common.h"
#include "Tasks.h"
#include "MotorController.h"


void main(void) {
    static OS_TCB Task1TCB;
    static CPU_STK Task1Stk[128];

    OS_ERR err;
    OSInit(&err);

    // Initialized with error
    if (err != OS_ERR_NONE) {
        printf("OS error code %d\n", err);
    }

    OSSemCreate(
        (OS_SEM*) &FaultState_Sem4,
        (CPU_CHAR*) "Fault State Semaphore",
        (OS_SEM_CTR) 0,
        (OS_ERR*) &err
    );

    if(err != OS_ERR_NONE){
        printf("Fault semaphore create failed:\n");
        printf(err);
    }

    OSTaskCreate( //create readCarCAN task
        (OS_TCB*)&ReadCarCAN_TCB,
        (CPU_CHAR*)"Read Car CAN",
        (OS_TASK_PTR)Task_ReadCarCAN,
        (void*)NULL,
        (OS_PRIO)1,
        (CPU_STK*)ReadCarCAN_Stk,
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
