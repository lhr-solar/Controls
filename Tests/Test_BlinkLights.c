/**
 * Test file for BlinkLights thread
 * 
 * 
 */ 
#include "common.h"
#include "config.h"
#include "BlinkLights.h"
#include "Tasks.h"

OS_TCB TaskInit_TCB;
CPU_STK TaskInit_Stk[256];

void Task_Init(void* p_arg) {
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);

    OS_ERR err;

    OSTaskCreate(&ReadSwitches_TCB,
                "Read Switches",
                Task_ReadSwitches,
                (void *)0,
                TASK_READ_SWITCHES_PRIO,
                ReadSwitches_Stk,
                WATERMARK_STACK_LIMIT,
                TASK_READ_SWITCHES_STACK_SIZE,
                0,
                0,
                (void *)0,
                OS_OPT_TASK_SAVE_FP | OS_OPT_TASK_STK_CHK,
                &err);
    while(err != OS_ERR_NONE);

    OSTaskCreate(&BlinkLight_TCB,
                "Blink Lights",
                Task_BlinkLight,
                (void *)0,
                TASK_BLINK_LIGHT_PRIO,
                BlinkLight_Stk,
                WATERMARK_STACK_LIMIT,
                TASK_BLINK_LIGHT_STACK_SIZE,
                0,
                0,
                (void *)0,
                OS_OPT_TASK_SAVE_FP | OS_OPT_TASK_STK_CHK,
                &err);
    while(err != OS_ERR_NONE);

    OSTaskDel(&TaskInit_TCB, &err);
}

int main(void) {
    OS_ERR err;

    OSInit(&err);

    while(err != OS_ERR_NONE);

    OSTaskCreate(&TaskInit_TCB,
                "Init Task",
                Task_Init,
                (void *)0,
                1,
                TaskInit_Stk,
                WATERMARK_STACK_LIMIT,
                DEFAULT_STACK_SIZE,
                0,
                0,
                (void *)0,
                OS_OPT_TASK_SAVE_FP | OS_OPT_TASK_STK_CHK,
                &err);
    
    while(err != OS_ERR_NONE);

    __enable_irq();

    OSStart(&err);

    while (1);
}
