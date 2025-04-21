#include "Tasks.h"
#include "common.h"
#include "os_cfg.h"
#include "Dashboard.h"
#include "StatusLeds.h"
#include "CANbus.h"
#include "CANConfig.h"
#include "Ignition.h"
#include "IOState.h"

/* Run in CAN_Loopback and use the IOState_recv task to recieve the IO_State CANbus message.
    - Controls fault LED indicates that the message was succesfully read
    - Use GDB to see if the message read is correct
   You should also read the message on a candapter
*/

// Allias for Controls fault led
#define IO_STATE_RECIEVE CONTROLS_FAULT_LED

void IOState_recv(void *p_arg){
    OS_ERR err;
    CANDATA_t msg;
    while(1){
        CANbus_Read(&msg, true, CARCAN);
        if(msg.data == IO_STATE){
            Status_Leds_Write(IO_STATE_RECIEVE, true);
        }
    }
}

void Task_Init(void *p_arg){
    OS_ERR err;

    // Start systick    
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);
    CANbus_Init(CARCAN, carCANFilterList, NUM_CARCAN_FILTERS);

    // Initialize IOState
    OSTaskCreate(
        (OS_TCB*)&IOState_TCB,
        (CPU_CHAR*)"PutIOState",
        (OS_TASK_PTR)Task_IOState,
        (void*)NULL,
        (OS_PRIO)TASK_PUT_IOSTATE_PRIO,
        (CPU_STK*)IOState_Stk,
        (CPU_STK_SIZE)WATERMARK_STACK_LIMIT,
        (CPU_STK_SIZE)TASK_SEND_CAR_CAN_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)0,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );
    assertOSError(err);

    OSTaskDel(NULL, &err);
}

int main(void){
    dashboardInit();
    Ignition_Init();
    OS_ERR err;
    OSInit(&err);
    assertOSError(err);

    // Initialize apps
    OSTaskCreate(
        (OS_TCB*)&Init_TCB,
        (CPU_CHAR*)"Init",
        (OS_TASK_PTR)Task_Init,
        (void*)NULL,
        (OS_PRIO)TASK_INIT_PRIO,
        (CPU_STK*)Init_Stk,
        (CPU_STK_SIZE)WATERMARK_STACK_LIMIT/10,
        (CPU_STK_SIZE)TASK_INIT_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)0,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR|OS_OPT_TASK_SAVE_FP),
        (OS_ERR*)&err
    );
    assertOSError(err);
    // Start OS
    OSStart(&err);
    assertOSError(err);

    while(1){
        // Should never reach here
    }
}