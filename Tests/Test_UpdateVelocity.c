#include "common.h"
#include "config.h"
#include "UpdateVelocity.h"

int main() {

    extern carState;
    OS_ERR err;
    OSInit(&err);

    if(err != OS_ERR_NONE){
        printf("OS error code %d\n", err);
    }

    OSTaskCreate(
        (OS_TCB*)&UpdateVelocity_TCB,
        (CPU_CHAR*)"UpdateVelocity",
        (OS_TASK_PTR)Task_UpdateVelocity,
        (void*) &carState,
        (OS_PRIO)1,
        (CPU_STK*)UpdateVelocity_Stk,
        (CPU_STK_SIZE)WATERMARK_STACK_LIMIT/10,
        (CPU_STK_SIZE)TASK_UPDATE_VELOCITY_STACK_SIZE,
        (OS_MSG_QTY)NULL,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );

    OS_CPU_SysTickInit();


    OSStart(&err);
    if (err != OS_ERR_NONE) {
        printf("OS error code %d\n", err);
    }
}