#include "ActivateArray.h"


static OS_TCB Task1TCB;
static CPU_STK Task1Stk[128];

void Task1(void *p_arg) {
    (void) p_arg;
    OS_ERR err;
    int time = 0;
    printf("Task1\n");
    while (1) {
        OSTimeDlyHMSM(0,0,1,0, OS_OPT_TIME_HMSM_STRICT, &err);
        printf("Time is %d\n", ++time);
        if (time == 5) OSSemPost(&ActivateArray_Sem4, OS_OPT_POST_1, &err);
    }
}

/**
 * Waits 5 seconds, then signals for the array to activate
 * Prints seconds to the screen
 * @param void
 * @return 
 */
int main(void) {
    OS_ERR err;
    OSInit(&err);

    if (err != OS_ERR_NONE) {
            printf("OS error code %d\n", err);
        }

    OSTaskCreate(
        (OS_TCB*)&ArrayConnection_TCB,
        (CPU_CHAR*)"ActivateArray",
        (OS_TASK_PTR)Task_ArrayConnection,
        (void*)NULL,
        (OS_PRIO)3,
        (CPU_STK*)ArrayConnection_Stk,
        (CPU_STK_SIZE)128/10,
        (CPU_STK_SIZE)128,
        (OS_MSG_QTY)NULL,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );



    if (err != OS_ERR_NONE) {
        printf("Task error code %d\n", err);
    }

    OSTaskCreate(
        (OS_TCB*)&Task1TCB,
        (CPU_CHAR*)"Task 1",
        (OS_TASK_PTR)Task1,
        (void*)NULL,
        (OS_PRIO)2,
        (CPU_STK*)Task1Stk,
        (CPU_STK_SIZE)128/10,
        (CPU_STK_SIZE)128,
        (OS_MSG_QTY)NULL,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );

    if (err != OS_ERR_NONE) {
        printf("Task error code %d\n", err);
    }

    OSSemCreate(&ActivateArray_Sem4, "Activate Array", 0, &err);
    OS_CPU_SysTickInit();

    OSStart(&err);
    if (err != OS_ERR_NONE) {
        printf("OS error code %d\n", err);
    }
    
}