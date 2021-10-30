/**
 * Test file for library to interact with Nextion display
 * 
 * To be tested on hardware
 */


#include "common.h"
#include "config.h"
#include "Display.h"
#include "Tasks.h"
#include "stm32f4xx.h"
#include <bsp.h>

static OS_TCB Task1_TCB;
static CPU_STK Task1_Stk[128];

void Task1(void *p_arg) {
    CPU_Init();
    // OS_CPU_SysTickInit();
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);
    OS_ERR err;
    CPU_TS ts;
    /*
    OSTaskCreate(
        (OS_TCB*)&SendDisplay_TCB,
        (CPU_CHAR*)"SendDisplay",
        (OS_TASK_PTR)Task_SendDisplay,
        (void*)p_arg,
        (OS_PRIO)TASK_SEND_DISPLAY_PRIO,
        (CPU_STK*)SendDisplay_Stk,
        (CPU_STK_SIZE)sizeof(SendDisplay_Stk)/10,
        (CPU_STK_SIZE)sizeof(SendDisplay_Stk),
        (OS_MSG_QTY)NULL,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR|OS_OPT_TASK_STK_CHK),
        (OS_ERR*)&err
    );
    */

    Display_Init();
    OSTimeDlyHMSM(0, 0, 5, 0, OS_OPT_TIME_HMSM_NON_STRICT, &err);
    Display_SetMainView();
    float vel = 13.7f;
    Display_SetVelocity(vel);
    while (1) {
        OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_NON_STRICT, &err);
        vel += ((random() % 5) - 2) * (-0.2f); // update velocity
    }
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
        (OS_MSG_QTY)NULL,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR|OS_OPT_TASK_STK_CHK),
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
