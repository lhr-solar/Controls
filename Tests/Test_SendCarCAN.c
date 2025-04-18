#include "SendCarCAN.h"
#include "CANConfig.h"
#include "StatusLeds.h"
#include "Tasks.h"
#include "Idle.h"


int main(){
    OS_ERR err;
    OSInit(&err);
    Status_Leds_Init();
    Status_Leds_Write(MOTOR_CONTACTOR_LED, ON);
    CANbus_Init(CARCAN, (CANId_t *) carCANFilterList, NUM_CARCAN_FILTERS);
    IdleInit();
    SendCarCAN_Init();
    TaskSwHook_Init();
        // Initialize SendCarCAN
        OSTaskCreate(
            (OS_TCB*)&SendCarCAN_TCB,
            (CPU_CHAR*)"SendCarCAN",
            (OS_TASK_PTR)Task_SendCarCAN,
            (void*)NULL,
            (OS_PRIO)TASK_SEND_CAR_CAN_PRIO,
            (CPU_STK*)SendCarCAN_Stk,
            (CPU_STK_SIZE)WATERMARK_STACK_LIMIT,
            (CPU_STK_SIZE)TASK_SEND_CAR_CAN_STACK_SIZE,
            (OS_MSG_QTY)0,
            (OS_TICK)0,
            (void*)NULL,
            (OS_OPT)(OS_OPT_TASK_STK_CLR|OS_OPT_TASK_SAVE_FP),
            (OS_ERR*)&err
        );
        assertOSError(err);
        OSStart(&err);
        assertOSError(err);
    while(1){
        // should never reach here
        Status_Leds_Write(OS_FAULT_LED, true);
    }

}