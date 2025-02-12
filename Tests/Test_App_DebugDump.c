/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "common.h"
#include "config.h"
#include "Tasks.h"
#include "stm32f4xx.h"
#include "CANbus.h"
#include "CANConfig.h"
#include "Contactors.h"
#include "Display.h"
#include "Minions.h"
#include "Pedals.h"
#include "CAN_Queue.h"
#include "UpdateDisplay.h"

#define IGN_CONT_PERIOD 100

int main(void) {
    // Disable interrupts
    __disable_irq();

    // Initialize some fault bitmaps for error checking purposes
    FaultBitmap = FAULT_NONE;

    OS_ERR err;
    OSInit(&err);
    OSSemCreate(&FaultState_Sem4, "Fault State Semaphore", 0, &err);

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
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );
    assertOSError(err);

    // Enable interrupts
    __enable_irq();

    // Start OS
    OSStart(&err);
    assertOSError(err);

    while(1);

    return 0;
}

void Task_Init(void *p_arg){
    OS_ERR err;

    // Start systick    
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);

    OSTimeDlyHMSM(0,0,5,0,OS_OPT_TIME_HMSM_STRICT,&err);

    assertOSError(err);
    
    // Initialize drivers
    Pedals_Init();
    OSTimeDlyHMSM(0,0,5,0,OS_OPT_TIME_HMSM_STRICT,&err);
    OSTimeDlyHMSM(0,0,10,0,OS_OPT_TIME_HMSM_STRICT,&err);
    BSP_UART_Init(UART_2);
    CANbus_Init(CARCAN, (CANId_t*)carCANFilterList, NUM_CARCAN_FILTERS);
    CANbus_Init(MOTORCAN, NULL, NUM_MOTORCAN_FILTERS);
    Contactors_Init();
    Display_Init();
    Minions_Init();
    CAN_Queue_Init();

    // Initialize applications
    UpdateDisplay_Init();

    /* Commented due to it messing up the test */
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
    assertOSError(err);

    // Initialize SendTritium
    OSTaskCreate(
        (OS_TCB*)&SendTritium_TCB,
        (CPU_CHAR*)"SendTritium",
        (OS_TASK_PTR)Task_SendTritium,
        (void*)NULL,
        (OS_PRIO)TASK_SEND_TRITIUM_PRIO,
        (CPU_STK*)SendTritium_Stk,
        (CPU_STK_SIZE)WATERMARK_STACK_LIMIT,
        (CPU_STK_SIZE)TASK_SEND_TRITIUM_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)0,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );
    assertOSError(err);

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
    assertOSError(err);

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
    assertOSError(err);

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
    assertOSError(err);

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
    assertOSError(err);

    // Initialize DebugDump
    OSTaskCreate(
        (OS_TCB*)&DebugDump_TCB,
        (CPU_CHAR*)"DebugDump",
        (OS_TASK_PTR)Task_DebugDump,
        (void*)NULL,
        (OS_PRIO)TASK_DEBUG_DUMP_PRIO,
        (CPU_STK*)DebugDump_Stk,
        (CPU_STK_SIZE)WATERMARK_STACK_LIMIT,
        (CPU_STK_SIZE)TASK_DEBUG_DUMP_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)0,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );
    assertOSError(err);

    while(1){
        Contactors_Set(MOTOR_CONTACTOR, Minions_Read(IGN_2), true); //turn on the contactor if the ign switch lets us
        assertOSError(err);
        OSTimeDlyHMSM(0, 0, 0, IGN_CONT_PERIOD, OS_OPT_TIME_HMSM_NON_STRICT, &err);
    }
}
