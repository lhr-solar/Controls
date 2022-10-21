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

    BSP_UART_Init(UART_2);
    CANbus_Init();
    Contactors_Init();
    Display_Init();
    Minions_Init();
    MotorController_Init(1.0f); // Let motor controller use 100% of bus current
    Pedals_Init();
    CAN_Queue_Init();

    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U) OSCfg_TickRate_Hz);
    OSTaskCreate(
        (OS_TCB*)&UpdateVelocity_TCB,
        (CPU_CHAR*)"UpdateVelocity",
        (OS_TASK_PTR)Task_UpdateVelocity,
        (void*) NULL,
        (OS_PRIO)TASK_UPDATE_VELOCITY_PRIO,
        (CPU_STK*)UpdateVelocity_Stk,
        (CPU_STK_SIZE)WATERMARK_STACK_LIMIT/10,
        (CPU_STK_SIZE)TASK_UPDATE_VELOCITY_STACK_SIZE,
        (OS_MSG_QTY) 0,
        (OS_TICK)NULL,
        (void*)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err
    );
    if (err != OS_ERR_NONE) {
        for(;;) x++;
    }

    OSSemCreate(&FaultState_Sem4, "Fault State Semaphore", 0, &err);
    if (err != OS_ERR_NONE) {
        for(;;) x++;
    }

    static bool lastCruiseEnPushed = true;
    bool cruiseEnablePushed = false;
    while (1) {
        OSTimeDlyHMSM(0,0,0,5,OS_OPT_TIME_HMSM_STRICT, &err);
        Switches_UpdateStates(); //UpdateVelocity thread depends on switch Values
        cruiseEnablePushed = Switches_Read(CRUZ_EN); //read cruise enable switch
        if (!lastCruiseEnPushed && cruiseEnablePushed) { //Rising Edge detector for updateVelocity thread
            UpdateVel_ToggleCruise = true;
        }
        lastCruiseEnPushed = cruiseEnablePushed;
        CANbuff buf;
        __unused
		ErrorStatus status = MotorController_Read(&buf);
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