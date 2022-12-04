#include "os.h"
#include "common.h"
#include "Tasks.h"
#include "CAN_Queue.h"
#include "Contactors.h"
#include "Minions.h"

OS_TCB Task_EnableContactorsTCB;
CPU_STK EnableContactorsStk[DEFAULT_STACK_SIZE];


// set up for the read car can task
void Task_EnableContactors(void *p_arg) {
    OS_ERR err;

    // enable systick and the other tasks
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);

    CANbus_Init();
    Contactors_Init();
    Contactors_Enable(ARRAY_CONTACTOR);
    Contactors_Enable(ARRAY_PRECHARGE);

    CAN_Queue_Init();

    Minions_Init();

    // delete this task
    OSTaskDel(&Task_EnableContactorsTCB, &err);
}

int main(void) {
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
        printf("%d",err);
    }

    OSTaskCreate( //create contactors enable task
        (OS_TCB*)&Task_EnableContactorsTCB,
        (CPU_CHAR*)"Enable Contactors",
        (OS_TASK_PTR)Task_EnableContactors,
        (void*)NULL,
        (OS_PRIO)1,
        (CPU_STK*)EnableContactorsStk,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE/10,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );

    OSTaskCreate( //create readCarCAN task
        (OS_TCB*)&ReadCarCAN_TCB,
        (CPU_CHAR*)"Read Car CAN",
        (OS_TASK_PTR)Task_ReadCarCAN,
        (void*)NULL,
        (OS_PRIO)12,
        (CPU_STK*)ReadCarCAN_Stk,
        (CPU_STK_SIZE)128/10,
        (CPU_STK_SIZE)128,
        (OS_MSG_QTY)0,
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

    while(1){}
}
