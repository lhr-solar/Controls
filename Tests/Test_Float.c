/**
 * Test file for testing floating point register restoration on context switch
 * 
 * To be tested on hardware
 */


#include "common.h"
#include "config.h"
#include "stm32f4xx.h"
#include <bsp.h>

static OS_TCB Task1_TCB;
static CPU_STK Task1_Stk[128];
static OS_TCB Task2_TCB;
static CPU_STK Task2_Stk[128];

void Task2(void *p_arg) {
    OS_ERR err;

    volatile float x = 7.8f;
    x *= 1.5f;
    x -= 8.7f;
    OSTimeDlyHMSM(0, 0, 2, 0, OS_OPT_TIME_HMSM_STRICT, &err);
    x /= 3.0f;
    OSTimeDlyHMSM(1, 0, 0, 0, OS_OPT_TIME_HMSM_STRICT, &err);
}

void Task1(void *p_arg) {
    CPU_Init();
    // OS_CPU_SysTickInit();
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);
    OS_ERR err;

    OSTaskCreate(&Task2_TCB, "Task2", Task2, NULL, 5, Task2_Stk, 64, 128, 0, 0, NULL, OS_OPT_TASK_SAVE_FP|OS_OPT_TASK_STK_CHK, &err);
    volatile float x = 3.7f;
    x *= 2.0f;
    x += 8.7f;
    OSTimeDlyHMSM(0, 0, 2, 0, OS_OPT_TIME_HMSM_STRICT, &err);
    x /= 1.5f;
    OSTimeDlyHMSM(1, 0, 0, 0, OS_OPT_TIME_HMSM_STRICT, &err);
   
}

int main(void) {
    OS_ERR err;
    OSInit(&err);
    if(err != OS_ERR_NONE){
        printf("OS error code %d\n",err);
    }
    OSTaskCreate(
        (OS_TCB*)&Task1_TCB,
        (CPU_CHAR*)"Task1",
        (OS_TASK_PTR)Task1,
        (void*)NULL,
        (OS_PRIO)4,
        (CPU_STK*)Task1_Stk,
        (CPU_STK_SIZE)sizeof(Task1_Stk)/10,
        (CPU_STK_SIZE)sizeof(Task1_Stk),
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_SAVE_FP|OS_OPT_TASK_STK_CHK),
        (OS_ERR*)&err
    );

    if (err != OS_ERR_NONE) {
        printf("Task1 error code %d\n", err);
    }
    OSStart(&err);
    if (err != OS_ERR_NONE) {
        printf("OS error code %d\n", err);
    }

    return 0;
}