/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file main.c
 * @brief 
 * 
 */

// (TODO) Status LEDs

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
#include "UpdateDisplay.h"
#include "SendCarCAN.h"
#include "daybreak_pins.h"
#include "BSP_GPIO.h"

int idle_time_ctr = 0;
int last_tick_cnt = 0;
int current_tick_cnt = 0;

void IdleTaskHook(void)
{
    static OS_ERR err;
    static bool toggle = false;
    while(1){
        current_tick_cnt = OSTimeGet(&err);
        
        if(last_tick_cnt != current_tick_cnt){
            idle_time_ctr++;
            last_tick_cnt = current_tick_cnt;

            if(current_tick_cnt % 50 == 0){
                BSP_GPIO_Write_Pin(HEARTBEAT_PORT, HEARTBEAT, toggle);
                toggle = !toggle;
            }
        }
    }
}

void IdleInit(void)
{
    BSP_GPIO_Init(HEARTBEAT_PORT, HEARTBEAT, OUTPUT, false);
    OS_AppIdleTaskHookPtr = &IdleTaskHook;
}

int main(void) {
    // Disable interrupts
    __disable_irq();


    OS_ERR err;
    OSInit(&err);
    IdleInit();
    TaskSwHook_Init();

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

    // Enable interrupts
    __enable_irq();

    // Start OS
    OSStart(&err);
    assertOSError(err);

    while(1);
}

void Task_Init(void *p_arg){
    OS_ERR err;

    // Start systick    
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);
    
    // Initialize drivers
    Pedals_Init();
    BSP_UART_Init(UART_2);
    CANbus_Init(CARCAN, carCANFilterList, NUM_CARCAN_FILTERS);
    CANbus_Init(MOTORCAN, NULL, NUM_MOTORCAN_FILTERS);
    Contactors_Init();
    Display_Init();
    Minions_Init();

    // Initialize applications
    UpdateDisplay_Init();
    SendCarCAN_Init();

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
        (OS_OPT)(OS_OPT_TASK_STK_CLR|OS_OPT_TASK_SAVE_FP),
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
        (OS_OPT)(OS_OPT_TASK_STK_CLR|OS_OPT_TASK_SAVE_FP),
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
        (OS_OPT)(OS_OPT_TASK_STK_CLR|OS_OPT_TASK_SAVE_FP),
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
        (OS_OPT)(OS_OPT_TASK_STK_CLR|OS_OPT_TASK_SAVE_FP),
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
        (OS_OPT)(OS_OPT_TASK_STK_CLR|OS_OPT_TASK_SAVE_FP),
        (OS_ERR*)&err
    );
    assertOSError(err);

    OSTaskDel(NULL, &err);
}

void HardFault_Handler(){
    __disable_irq();
    EmergencyContactorOpen();
    while(1){}
}

void MemManage_Handler(){
    __disable_irq();
    EmergencyContactorOpen();
    while(1){}
}

void BusFault_Handler(){
    __disable_irq();
    EmergencyContactorOpen();
    while(1){}
}

void UsageFault_Handler(){
    __disable_irq();
    EmergencyContactorOpen();
    while(1){}
}