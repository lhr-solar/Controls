#include "BlinkerLight.h"

int main(void){
    OS_ERR err;
    OSInit(&err);

    if(err != OS_ERR_NONE){
        printf("OS error code %d\n", err);
    }
    
    OSTaskCreate(
        (OS_TCB*)&ReadSwitches_TCB,
        (CPU_CHAR*)"ReadSwitches",
        (OS_TASK_PTR)Task_ReadSwitches,
        (void*)NULL,
        (OS_PRIO)6,
        (CPU_STK*)ReadSwitches_Stk,
        (CPU_STK_SIZE)WATERMARK_STACK_LIMIT/10,
        (CPU_STK_SIZE)TASK_READ_SWITCHES_STACK_SIZE,
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
        (OS_TCB*)&BlinkLight_TCB,
        (CPU_CHAR*)"BlinkerLights",
        (OS_TASK_PTR)Task_BlinkLight,
        (void*)NULL,
        (OS_PRIO)9,
        (CPU_STK*)BlinkLight_Stk,
        (CPU_STK_SIZE)WATERMARK_STACK_LIMIT/10,
        (CPU_STK_SIZE)TASK_BLINK_LIGHT_STACK_SIZE,
        (OS_MSG_QTY)NULL,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );

    OSSemCreate(&BlinkLight_Sem4, "Blink Lights", 0, &err);
    OS_CPU_SysTickInit();


    OSStart(&err);
    if (err != OS_ERR_NONE) {
        printf("OS error code %d\n", err);
    }
}