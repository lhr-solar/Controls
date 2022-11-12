#include "common.h"
#include "config.h"
#include "UpdateVelocity.h"
#include "BSP_UART.h"
#include "CANbus.h"
#include "Display.h"
#include "Contactors.h"
#include "Minions.h"
#include "MotorController.h"
#include "Pedals.h"
#include "CAN_Queue.h"

static volatile int x = 0;

OS_TCB Task1_TCB;
CPU_STK Task1_Stk[256];

void Task1(void *p_arg) {
    OS_ERR err;

    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);
    Pedals_Init();
    OSTimeDlyHMSM(0,0,5,0,OS_OPT_TIME_HMSM_STRICT,&err);
    MotorController_Init(1.0f); // Let motor controller use 100% of bus current
    OSTimeDlyHMSM(0,0,10,0,OS_OPT_TIME_HMSM_STRICT,&err);
    BSP_UART_Init(UART_2);
    CANbus_Init();
    Contactors_Init();
    Display_Init();
    Minions_Init();
    CAN_Queue_Init();

    //init updatevelocity
    OSTaskCreate(
        (OS_TCB*)&SendTritium_TCB,
        (CPU_CHAR*)"UpdateVelocity",
        (OS_TASK_PTR)Task_SendTritium,
        (void*) NULL,
        (OS_PRIO)TASK_SEND_TRITIUM_PRIO,
        (CPU_STK*)SendTritium_Stk,
        (CPU_STK_SIZE)WATERMARK_STACK_LIMIT/10,
        (CPU_STK_SIZE)TASK_SEND_TRITIUM_STACK_SIZE,
        (OS_MSG_QTY) 0,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );
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


    if (err != OS_ERR_NONE) {
        for(;;) x++;
    }

    OSSemCreate(&FaultState_Sem4, "Fault State Semaphore", 0, &err);
    if (err != OS_ERR_NONE) {
        for(;;) x++;
    }

    //value injection for display
    SupplementalVoltage = 2000;
    StateOfCharge = 100 * 1000000;


    while (1) {
        OSTimeDlyHMSM(0,0,0,10,OS_OPT_TIME_HMSM_STRICT, &err);
        // SupplementalVoltage += 100; //value injection for display thread, normally would be handled by ReadCarCAN thread

    }
}

int main() {
    OS_ERR err;
    OSInit(&err);

    if(err != OS_ERR_NONE){
        for (;;) x++;
    }

    OSTaskCreate(&Task1_TCB,
                "Task 1",
                Task1,
                (void *) NULL,
                5,
                Task1_Stk,
                16,
                256,
                0,
                0,
                (void *) NULL,
                OS_OPT_TASK_STK_CHK,
                &err
    );

    if (err != OS_ERR_NONE) {
        for(;;) x++;
    }

    OSStart(&err);
    if (err != OS_ERR_NONE) {
        for(;;) x++;
    }
}