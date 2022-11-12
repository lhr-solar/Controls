/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "common.h"
#include "config.h"
#include "Tasks.h"
#include "stm32f4xx.h"
#include "CANbus.h"
#include "Contactors.h"
#include "Display.h"
#include "Minions.h"
#include "MotorController.h"
#include "Pedals.h"
#include "CAN_Queue.h"

#include "UpdateDisplay.h"

int main(void) {
    // Disable interrupts
    __disable_irq();

    // Initialize some fault bitmaps for error checking purposes
    OSErrLocBitmap = OS_NONE_LOC;
    FaultBitmap = FAULT_NONE;

    OS_ERR err;
    OSInit(&err);
    OSSemCreate(&FaultState_Sem4, "Fault State Semaphore", 0, &err);

    assertOSError(OS_MAIN_LOC, err);

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
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );
    assertOSError(OS_MAIN_LOC, err);

    // Enable interrupts
    __enable_irq();

    // Start OS
    OSStart(&err);
    assertOSError(OS_MAIN_LOC, err);

    while(1);

    return 0;
}

void Task_Init(void *p_arg){
    OS_ERR err;

    // Start systick    
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);

    OSTimeDlyHMSM(0,0,5,0,OS_OPT_TIME_HMSM_STRICT,&err);

    assertOSError(OS_MAIN_LOC, err);
    
    // Initialize drivers
    Pedals_Init();
    OSTimeDlyHMSM(0,0,5,0,OS_OPT_TIME_HMSM_STRICT,&err);
    MotorController_Init(1.0f); // Let motor controller use 100% of bus current
    OSTimeDlyHMSM(0,0,10,0,OS_OPT_TIME_HMSM_STRICT,&err);
    BSP_UART_Init(UART_2);
    CANbus_Init(CARCAN);
    CANbus_Init(MOTORCAN);
    Contactors_Init();
    Display_Init();
    Minion_Init();
    CAN_Queue_Init();

    // Initialize applications
    UpdateDisplay_Init();

    // Initialize FaultState
    OSTaskCreate(
        (OS_TCB*)&FaultState_TCB,
        (CPU_CHAR*)"FaultState",
        (OS_TASK_PTR)Task_FaultState,
        (void*)NULL,
        (OS_PRIO)TASK_FAULT_STATE_PRIO,
        (CPU_STK*)FaultState_Stk,
        (CPU_STK_SIZE)WATERMARK_STACK_LIMIT,
        (CPU_STK_SIZE)TASK_FAULT_STATE_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)0,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );
    assertOSError(OS_MAIN_LOC, err);

    // Initialize UpdateVelocity
    OSTaskCreate(
        (OS_TCB*)&UpdateVelocity_TCB,
        (CPU_CHAR*)"UpdateVelocity",
        (OS_TASK_PTR)Task_UpdateVelocity,
        (void*)NULL,
        (OS_PRIO)TASK_UPDATE_VELOCITY_PRIO,
        (CPU_STK*)UpdateVelocity_Stk,
        (CPU_STK_SIZE)WATERMARK_STACK_LIMIT,
        (CPU_STK_SIZE)TASK_UPDATE_VELOCITY_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)0,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );
    assertOSError(OS_MAIN_LOC, err);

    // Initialize ReadCarCAN
    OSTaskCreate(
        (OS_TCB*)&ReadCarCAN_TCB,
        (CPU_CHAR*)"ReadCarCAN",
        (OS_TASK_PTR)Task_ReadCarCAN,
        (void*)NULL,
        (OS_PRIO)TASK_READ_CAR_CAN_PRIO,
        (CPU_STK*)ReadCarCAN_Stk,
        (CPU_STK_SIZE)WATERMARK_STACK_LIMIT,
        (CPU_STK_SIZE)TASK_READ_CAR_CAN_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)0,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );
    assertOSError(OS_MAIN_LOC, err);

    // Initialize UpdateDisplay
    OSTaskCreate(
        (OS_TCB*)&UpdateDisplay_TCB,
        (CPU_CHAR*)"UpdateDisplay",
        (OS_TASK_PTR)Task_UpdateDisplay,
        (void*)NULL,
        (OS_PRIO)TASK_UPDATE_DISPLAY_PRIO,
        (CPU_STK*)UpdateDisplay_Stk,
        (CPU_STK_SIZE)WATERMARK_STACK_LIMIT,
        (CPU_STK_SIZE)TASK_UPDATE_DISPLAY_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)0,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );
    assertOSError(OS_MAIN_LOC, err);

    // Initialize ReadTritium
    OSTaskCreate(
        (OS_TCB*)&ReadTritium_TCB,
        (CPU_CHAR*)"ReadTritium",
        (OS_TASK_PTR)Task_ReadTritium,
        (void*)NULL,
        (OS_PRIO)TASK_READ_TRITIUM_PRIO,
        (CPU_STK*)ReadTritium_Stk,
        (CPU_STK_SIZE)WATERMARK_STACK_LIMIT,
        (CPU_STK_SIZE)TASK_READ_TRITIUM_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)0,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );
    assertOSError(OS_MAIN_LOC, err);

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
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );
    assertOSError(OS_MAIN_LOC, err);

    // Initialize IgnitionContactor
    OSTaskCreate(
        (OS_TCB*)&IgnCont_TCB,
        (CPU_CHAR*)"IgnitionContactor",
        (OS_TASK_PTR)Task_Contactor_Ignition,
        (void*)NULL,
        (OS_PRIO)TASK_IGN_CONT_PRIO,
        (CPU_STK*)IgnCont_Stk,
        (CPU_STK_SIZE)WATERMARK_STACK_LIMIT,
        (CPU_STK_SIZE)TASK_IGN_CONT_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)0,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );
    assertOSError(OS_MAIN_LOC, err);

    while(1){
        OSTimeDlyHMSM(1,0,0,0,OS_OPT_TIME_HMSM_STRICT,&err);
    }
}
