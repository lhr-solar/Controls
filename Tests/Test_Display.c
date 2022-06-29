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
    // CPU_TS ts;
    OSTaskCreate(
        (OS_TCB *)&SendDisplay_TCB,
        (CPU_CHAR *)"SendDisplay_TCB",
        (OS_TASK_PTR)Task_SendDisplay,
        (void *)NULL,
        (OS_PRIO)13,
        (CPU_STK *)SendDisplay_Stk,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE / 10,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void *)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR *)&err);
   
    Display_Init();
    OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_NON_STRICT, &err);
    Display_SetMainView();
    float vel = 0.0f;
    SupplementalVoltage = 200;
    StateOfCharge = 100000;
    RegenEnable = ON;
    State cr = ON;
    Display_SetVelocity(vel);
    while (1) {
        OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_NON_STRICT, &err);
        vel += 0.1; // update velocity
        SupplementalVoltage += 100;
        StateOfCharge += 10000;
        if (vel > 30.0f) vel = 0.0f;
        cr = (cr==ON)?OFF:ON;
        RegenEnable = (RegenEnable==ON)?OFF:ON;
        //Display_SetVelocity(vel);
        //Display_SetSBPV(SupplementalVoltage);
        //Display_SetChargeState(StateOfCharge);
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