#include "common.h"
#include "config.h"
#include "UpdateVelocity.h"

static volatile int x = 0;

OS_TCB Task1_TCB;
CPU_STK Task1_Stk[256];

void Task1(void *p_arg) {
    OS_ERR err;
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);
    OSTaskCreate(
        (OS_TCB*)&UpdateVelocity_TCB,
        (CPU_CHAR*)"UpdateVelocity",
        (OS_TASK_PTR)Task_UpdateVelocity,
        (void*) NULL,
        (OS_PRIO)TASK_UPDATE_VELOCITY_PRIO,
        (CPU_STK*)UpdateVelocity_Stk,
        (CPU_STK_SIZE)WATERMARK_STACK_LIMIT/10,
        (CPU_STK_SIZE)TASK_UPDATE_VELOCITY_STACK_SIZE,
        (OS_MSG_QTY) 0,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );
    while (1) {
        OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT, &err);
    }
}

int main() {
    OS_ERR err;
    OSInit(&err);

    if(err != OS_ERR_NONE){
        for (;;) x++;
    }

    OSTaskCreate(&Task1_TCB,
    "Task 1",
    Task1,
    (void *) NULL,
    5,
    Task1_Stk,
    16,
    256,
    0,
    0,
    (void *) NULL,
    OS_OPT_TASK_STK_CHK,
    &err
    );

    if (err != OS_ERR_NONE) {
        for(;;) x++;
    }

    OSStart(&err);
    if (err != OS_ERR_NONE) {
        for(;;) x++;
    }
}