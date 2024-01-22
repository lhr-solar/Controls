/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file main.c
 * @brief
 *
 */

#include "BSP_UART.h"
#include "CanBus.h"
#include "CanConfig.h"
#include "Contactors.h"
#include "Display.h"
#include "Minions.h"
#include "Pedals.h"
#include "SendCarCan.h"
#include "Tasks.h"
#include "UpdateDisplay.h"
#include "common.h"
#include "config.h"
#include "stm32f4xx.h"

int main(void) {
    // Disable interrupts
    __disable_irq();

    OS_ERR err = 0;
    OSInit(&err);
    TaskSwHookInit();

    ASSERT_OS_ERROR(err);

    // Initialize apps
    OSTaskCreate((OS_TCB *)&init_tcb, (CPU_CHAR *)"Init", (OS_TASK_PTR)TaskInit,
                 (void *)NULL, (OS_PRIO)TASK_INIT_PRIO, (CPU_STK *)init_stk,
                 (CPU_STK_SIZE)WATERMARK_STACK_LIMIT / 10,  // NOLINT
                 (CPU_STK_SIZE)TASK_INIT_STACK_SIZE, (OS_MSG_QTY)0, (OS_TICK)0,
                 (void *)NULL,
                 (OS_OPT)(OS_OPT_TASK_STK_CLR | OS_OPT_TASK_SAVE_FP),
                 (OS_ERR *)&err);
    ASSERT_OS_ERROR(err);

    // Enable interrupts
    __enable_irq();

    // Start OS
    OSStart(&err);
    ASSERT_OS_ERROR(err);

    while (1) {}
}

void TaskInit(void *p_arg) {
    OS_ERR err = 0;

    // Start systick
    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U)OSCfg_TickRate_Hz);

    // Initialize drivers
    PedalsInit();
    BspUartInit(kUart2);
    CanBusInit(CARCAN, car_can_filter_list, NUM_CARCAN_FILTERS);
    CanBusInit(MOTORCAN, NULL, NUM_MOTORCAN_FILTERS);
    ContactorsInit();
    DisplayInit();
    MinionsInit();

    // Initialize applications
    UpdateDisplayInit();
    SendCarCanInit();

    // Initialize ReadTritium
    OSTaskCreate((OS_TCB *)&read_tritium_tcb, (CPU_CHAR *)"ReadTritium",
                 (OS_TASK_PTR)TaskReadTritium, (void *)NULL,
                 (OS_PRIO)TASK_READ_TRITIUM_PRIO, (CPU_STK *)read_tritium_stk,
                 (CPU_STK_SIZE)WATERMARK_STACK_LIMIT,
                 (CPU_STK_SIZE)TASK_READ_TRITIUM_STACK_SIZE, (OS_MSG_QTY)0,
                 (OS_TICK)0, (void *)NULL,
                 (OS_OPT)(OS_OPT_TASK_STK_CLR | OS_OPT_TASK_SAVE_FP),
                 (OS_ERR *)&err);
    ASSERT_OS_ERROR(err);

    // Initialize SendTritium
    OSTaskCreate((OS_TCB *)&send_tritium_tcb, (CPU_CHAR *)"SendTritium",
                 (OS_TASK_PTR)TaskSendTritium, (void *)NULL,
                 (OS_PRIO)TASK_SEND_TRITIUM_PRIO, (CPU_STK *)send_tritium_stk,
                 (CPU_STK_SIZE)WATERMARK_STACK_LIMIT,
                 (CPU_STK_SIZE)TASK_SEND_TRITIUM_STACK_SIZE, (OS_MSG_QTY)0,
                 (OS_TICK)0, (void *)NULL,
                 (OS_OPT)(OS_OPT_TASK_STK_CLR | OS_OPT_TASK_SAVE_FP),
                 (OS_ERR *)&err);
    ASSERT_OS_ERROR(err);

    // Initialize ReadCarCAN
    OSTaskCreate((OS_TCB *)&read_car_can_tcb, (CPU_CHAR *)"ReadCarCAN",
                 (OS_TASK_PTR)TaskReadCarCan, (void *)NULL,
                 (OS_PRIO)TASK_READ_CAR_CAN_PRIO, (CPU_STK *)read_car_can_stk,
                 (CPU_STK_SIZE)WATERMARK_STACK_LIMIT,
                 (CPU_STK_SIZE)TASK_READ_CAR_CAN_STACK_SIZE, (OS_MSG_QTY)0,
                 (OS_TICK)0, (void *)NULL,
                 (OS_OPT)(OS_OPT_TASK_STK_CLR | OS_OPT_TASK_SAVE_FP),
                 (OS_ERR *)&err);
    ASSERT_OS_ERROR(err);

    // Initialize UpdateDisplay
    OSTaskCreate(
        (OS_TCB *)&update_display_tcb, (CPU_CHAR *)"UpdateDisplay",
        (OS_TASK_PTR)TaskUpdateDisplay, (void *)NULL,
        (OS_PRIO)TASK_UPDATE_DISPLAY_PRIO, (CPU_STK *)update_display_stk,
        (CPU_STK_SIZE)WATERMARK_STACK_LIMIT,
        (CPU_STK_SIZE)TASK_UPDATE_DISPLAY_STACK_SIZE, (OS_MSG_QTY)0, (OS_TICK)0,
        (void *)NULL, (OS_OPT)(OS_OPT_TASK_STK_CLR | OS_OPT_TASK_SAVE_FP),
        (OS_ERR *)&err);
    ASSERT_OS_ERROR(err);

    // Initialize SendCarCAN
    OSTaskCreate((OS_TCB *)&send_car_can_tcb, (CPU_CHAR *)"SendCarCAN",
                 (OS_TASK_PTR)TaskSendCarCan, (void *)NULL,
                 (OS_PRIO)TASK_SEND_CAR_CAN_PRIO, (CPU_STK *)send_car_can_stk,
                 (CPU_STK_SIZE)WATERMARK_STACK_LIMIT,
                 (CPU_STK_SIZE)TASK_SEND_CAR_CAN_STACK_SIZE, (OS_MSG_QTY)0,
                 (OS_TICK)0, (void *)NULL,
                 (OS_OPT)(OS_OPT_TASK_STK_CLR | OS_OPT_TASK_SAVE_FP),
                 (OS_ERR *)&err);
    ASSERT_OS_ERROR(err);

    OSTaskDel(NULL, &err);
}
