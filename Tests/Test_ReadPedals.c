/**
 * 
 * Test file for RTOS task read pedals
 * 
 */
#include "RTOSPedals.h"

#define INITIAL_SWITCH_STATES {OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF}
#define INITIAL_BLINKER_STATES {OFF, OFF, OFF}

car_state_t carState = {0.0f, 0.0f, 0.0f, 0.0f, 0, 0, 0, INITIAL_SWITCH_STATES, INITIAL_BLINKER_STATES, OFF, OFF};

int main(void){
    OS_ERR err;
    OSInit(&err);

    if(err != OS_ERR_NONE){
        printf("OS error code %d\n", err);
    }

    OSTaskCreate(
        (OS_TCB*)&ReadPedals_TCB,
        (CPU_CHAR*)"ReadPedals",
        (OS_TASK_PTR)Task_ReadPedals,
        (void*)&carState,
        (OS_PRIO)TASK_READ_PEDALS_PRIO,
        (CPU_STK*)ReadPedals_Stk,
        (CPU_STK_SIZE)WATERMARK_STACK_LIMIT/10,
        (CPU_STK_SIZE)TASK_READ_PEDALS_STACK_SIZE,
        (OS_MSG_QTY)NULL,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );

    if(err != OS_ERR_NONE){
        printf("OS error code %d\n", err);
    }

    OS_CPU_SysTickInit();

    OSStart(&err);
    if (err != OS_ERR_NONE) {
        printf("OS error code %d\n", err);
    }
}
