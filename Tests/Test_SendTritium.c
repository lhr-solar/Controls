#include "common.h"
#include "config.h"
#include "BSP_UART.h"
#include "CANbus.h"
#include "Display.h"
#include "Contactors.h"
#include "Minions.h"
#include "MotorController.h"
#include "Pedals.h"
#include "CAN_Queue.h"
#include "ReadCarCAN.h"
//#include "SendTritium.c"

#define __TEST_SENDTRITIUM
// Inputs
extern bool cruiseEnable;
extern bool cruiseSet;
extern bool regenToggle;
extern uint8_t brakePedalPercent;
extern uint8_t accelPedalPercent;
extern bool reverseSwitch;
extern bool forwardSwitch;

// Current State
//extern TritiumStateName_e state;



static OS_TCB Task1TCB;
static CPU_STK Task1Stk[DEFAULT_STACK_SIZE];

void Task1(void *arg)
{
    CPU_Init();
    Contactors_Init();
   Display_Init();
    CANbus_Init();
   // Minions_Init();
    MotorController_Init(1.0f);
    Pedals_Init();
    CAN_Queue_Init();
    BSP_UART_Init(UART_2);

    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U)OSCfg_TickRate_Hz);
    SupplementalVoltage = 200;
    StateOfCharge = 12345678;
    RegenEnable = ON;
    Contactors_Enable(ARRAY_CONTACTOR);
    Contactors_Set(ARRAY_CONTACTOR, ON);

    OS_ERR err;

    OSTaskCreate(
        (OS_TCB*)&SendTritium_TCB,
        (CPU_CHAR*)"SendTritium",
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

// Normal Drive
//printf(state);

// Normal Drive
accelPedalPercent = 45;
brakePedalPercent = 1;
cruiseSet = false;
cruiseEnable = false;
//printf(state);

// Record Velocity
accelPedalPercent = 45;
brakePedalPercent = 1;
cruiseSet = true;
cruiseEnable = true;
printf("accelPedalPercent");

    
    
    /*
    switches_t(CRUZ_EN) = 1;
        switches_t(CRUZ_SW) = 1;
        brakePedalPercent;
        printf("Print out");
    */    
        
  
        // Testing SendTritium directly
        /*Display_SetSBPV(SupplementalVoltage);
        Display_SetChargeState(StateOfCharge);
        Display_SetRegenEnabled(RegenEnable);
        Display_SetVelocity(25.0);*/

    while (1)
    {
        OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &err);
        assertOSError(OS_MAIN_LOC, err);

        SupplementalVoltage += 100;
        StateOfCharge += 100000;
     
    
        }
};

int main()
{
    OS_ERR err;
    OSInit(&err);
    OSSemCreate( // create fault sem4
        &FaultState_Sem4,
        "Fault State Semaphore",
        0,
        &err);
    assertOSError(OS_MAIN_LOC, err);

    OSTaskCreate( // create fault task
        (OS_TCB *)&FaultState_TCB,
        (CPU_CHAR *)"Fault State",
        (OS_TASK_PTR)&Task_FaultState,
        (void *)NULL,
        (OS_PRIO)1,
        (CPU_STK *)FaultState_Stk,
        (CPU_STK_SIZE)128 / 10,
        (CPU_STK_SIZE)128,
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void *)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR *)&err);
    assertOSError(OS_MAIN_LOC, err);

    // create tester thread
    OSTaskCreate(
        (OS_TCB *)&Task1TCB,
        (CPU_CHAR *)"Task 1",
        (OS_TASK_PTR)Task1,
        (void *)NULL,
        (OS_PRIO)13,
        (CPU_STK *)Task1Stk,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE / 10,
        (CPU_STK_SIZE)DEFAULT_STACK_SIZE,
        (OS_MSG_QTY)0,
        (OS_TICK)NULL,
        (void *)NULL,
        (OS_OPT)(OS_OPT_TASK_STK_CLR),
        (OS_ERR *)&err);
    assertOSError(OS_MAIN_LOC, err);

    OSStart(&err);
}