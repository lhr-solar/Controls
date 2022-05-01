/**
 * Test file for SendDisplay task
 * 
 * To be tested on hardware
 */


#include "common.h"
#include "config.h"
#include "Display.h"
#include "Tasks.h"
#include "stm32f4xx.h"
#include <bsp.h>

int main(void) {
    OS_ERR err;
    car_state_t car;

    OSInit(&err);
    if(err != OS_ERR_NONE){
        printf("OS error code %d\n",err);
    }

    // Initialize the Send Display task
    OSTaskCreate(
        (OS_TCB*)&SendDisplay_TCB,
        (CPU_CHAR*)"SendDisplay",
        (OS_TASK_PTR)Task_SendDisplay,
        (void*) car,
        (OS_PRIO)TASK_SEND_DISPLAY_PRIO,
        (CPU_STK*)SendDisplay_Stk,
        (CPU_STK_SIZE)sizeof(SendDisplay_Stk)/10,
        (CPU_STK_SIZE)sizeof(SendDisplay_Stk),
        (OS_MSG_QTY)NULL,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR|OS_OPT_TASK_STK_CHK),
        (OS_ERR*)&err

    if (err != OS_ERR_NONE) {
        printf("Task error code %d\n", err);
    }
    OSStart(&err);
    if (err != OS_ERR_NONE) {
        printf("OS error code %d\n", err);
    }

    while (1) {
        car.CurrentVelocity = (random() % 100) / 10.0f;
        car.CruiseControlEnable = random() % 2;
        car.CRSet = random() % 3;
        car.MotorErrorCode.motorTempErr = random() % 2;
        car.ErrorCode.SendCANErr = random() % 2;
        OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_NON_STRICT, &err);
    }

    return 0;
}