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

int main(void) {
    // Disable interrupts
    __disable_irq();

    // Initialize some fault bitmaps for error checking purposes
    OSErrLocBitmap = OS_NONE_LOC;
    FaultBitmap = FAULT_NONE;

    OS_ERR err;
    OSInit(&err);
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

    return 0;
}

void Task_Init(void *p_arg){
    OS_ERR err;

    // Start systick    
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);

    // Create FaultState semaphore
    OSSemCreate(
        (OS_SEM*)&FaultState_Sem4,
        (CPU_CHAR*)"FaultState Semaphore",
        (OS_SEM_CTR)0,
        (OS_ERR*)&err
    );
    assertOSError(OS_MAIN_LOC, err);

    // Create DisplayChange semaphore
    OSSemCreate(
        (OS_SEM*)&DisplayChange_Sem4,
        (CPU_CHAR*)"DisplayChange Semaphore",
        (OS_SEM_CTR)0,
        (OS_ERR*)&err
    );
    assertOSError(OS_MAIN_LOC, err);

    // Create CarCAN semaphore
    OSSemCreate(
        (OS_SEM*)&CarCAN_Sem4,
        (CPU_CHAR*)"CarCAN Semaphore",
        (OS_SEM_CTR)0,
        (OS_ERR*)&err
    );
    assertOSError(OS_MAIN_LOC, err);

    // Create ReadTritium semaphore
    OSSemCreate(
        (OS_SEM*)&ReadTritium_Sem4,
        (CPU_CHAR*)"ReadTritium Semaphore",
        (OS_SEM_CTR)0,
        (OS_ERR*)&err
    );
    assertOSError(OS_MAIN_LOC, err);

    // Create BlinkLights semaphore
    OSSemCreate(
        (OS_SEM*)&BlinkLight_Sem4,
        (CPU_CHAR*)"BlinkLights Semaphore",
        (OS_SEM_CTR)0,
        (OS_ERR*)&err
    );
    assertOSError(OS_MAIN_LOC, err);

    // Create SendCarCAN semaphore
    OSSemCreate(
        (OS_SEM*)&SendCarCAN_Sem4,
        (CPU_CHAR*)"SendCarCAN Semaphore",
        (OS_SEM_CTR)0,
        (OS_ERR*)&err
    );
    assertOSError(OS_MAIN_LOC, err);

    // Initialize drivers
    CANbus_Init();
    Contactors_Init();
    Display_Init();
    Minions_Init();
    MotorController_Init();
    Pedals_Init();

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

    // Initialize SendDisplay
    OSTaskCreate(
        (OS_TCB*)&SendDisplay_TCB,
        (CPU_CHAR*)"SendDisplay",
        (OS_TASK_PTR)Task_SendDisplay,
        (void*)NULL,
        (OS_PRIO)TASK_SEND_DISPLAY_PRIO,
        (CPU_STK*)SendDisplay_Stk,
        (CPU_STK_SIZE)WATERMARK_STACK_LIMIT,
        (CPU_STK_SIZE)TASK_SEND_DISPLAY_STACK_SIZE,
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

    // Initialize ReadSwitches
    OSTaskCreate(
        (OS_TCB*)&ReadSwitches_TCB,
        (CPU_CHAR*)"ReadSwitches",
        (OS_TASK_PTR)Task_ReadSwitches,
        (void*)NULL,
        (OS_PRIO)TASK_READ_SWITCHES_PRIO,
        (CPU_STK*)ReadSwitches_Stk,
        (CPU_STK_SIZE)WATERMARK_STACK_LIMIT,
        (CPU_STK_SIZE)TASK_READ_SWITCHES_STACK_SIZE,
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

    // Initialize BlinkLights
    OSTaskCreate(
        (OS_TCB*)&BlinkLight_TCB,
        (CPU_CHAR*)"BlinkLight",
        (OS_TASK_PTR)Task_BlinkLight,
        (void*)NULL,
        (OS_PRIO)TASK_BLINK_LIGHT_PRIO,
        (CPU_STK*)BlinkLight_Stk,
        (CPU_STK_SIZE)WATERMARK_STACK_LIMIT,
        (CPU_STK_SIZE)TASK_BLINK_LIGHT_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)0,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );
    assertOSError(OS_MAIN_LOC, err);

    // Delete initialization task
    OSTaskDel(
        (OS_TCB*)&Init_TCB,
        (OS_ERR*)&err
    );
}
