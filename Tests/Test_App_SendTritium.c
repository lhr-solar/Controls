#include "Minions.h"
#include "Pedals.h"
#include "CANbus.h"
#include "UpdateDisplay.h"
#include "ReadCarCAN.h"
#include "BSP_UART.h"
#include "Tasks.h"

#include "SendTritium.h"

static OS_TCB Task1TCB;
static CPU_STK Task1Stk[DEFAULT_STACK_SIZE];

void stateBuffer(){
    OS_ERR err;
    OSTimeDlyHMSM(0, 0, 0, 150, OS_OPT_TIME_HMSM_STRICT, &err);
    assertOSError(err);
}

void printFullState() {
    printf("-------------------\n\r");
    char stateName[20];
    switch (get_state())
    {
        case FORWARD_DRIVE:
            strcpy(stateName, "FORWARD_DRIVE");
            break;
        case PARK_STATE:
            strcpy(stateName, "PARK_STATE");
            break;
        case REVERSE_DRIVE:
            strcpy(stateName, "REVERSE_DRIVE");
            break;
        default:
            strcpy(stateName, "UNKNOWN");
            break;
    }
    char gearName[20];
    switch(get_gear()) {
        case FORWARD_GEAR:
            strcpy(gearName, "FWD_GEAR");
            break;
        case PARK_GEAR:
            strcpy(gearName, "PARK_GEAR");
            break;
        case REVERSE_GEAR:
            strcpy(gearName, "REV_GEAR");
            break;
        default:
            strcpy(gearName, "Unknown");
            break;    
    }
    printf("State: %s----------------------\n\r", stateName);
    printf("brakePedalPercent: %u\n\r", get_brakePedalPercent());
    printf("accelPedalPercent: %u\n\r", get_accelPedalPercent());
    printf("Gear: %s\n\r", gearName);
    print_float("currentSetpoint: \n\r", get_currentSetpoint());
    print_float("velocitySetpoint: \n\r", get_velocitySetpoint());
    print_float("velocityObserved: \n\r", get_velocityObserved());
    printf("----------------------------\n\r");
}


void Task1(void *arg)
{
    OS_ERR err;

    CPU_Init();
    BSP_UART_Init(USB);
    Pedals_Init();
    CANbus_Init(MOTORCAN, NULL, 0);
    Minions_Init();
    UpdateDisplay_Init();

    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U)OSCfg_TickRate_Hz);
    // set_regenEnable(ON);

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
    assertOSError(err);

    // Park to Forward Drive
    printf("\n\rTesting: Park -> Forward Drive==============\n\r");
    printf("ACTION: Switch to forward gear\n\r");
    printFullState();
    stateBuffer();
    printFullState();
    while(get_state() != FORWARD_DRIVE){}
    printf("============================================\n\r");

    // Forward Drive to Park Drive
    printf("\n\rTesting: Forward -> Park====================\n\r");
    printf("ACTION: Switch to reverse gear\n\r");
    printFullState();
    stateBuffer();
    printFullState();
    while(get_state() != PARK_STATE){}
    printf("ACTION: Switch to forward gear\n\r");
    printFullState();
    stateBuffer();
    printFullState();
    while(get_state() != FORWARD_DRIVE){}
    printf("ACTION: Switch to park gear\n\r");
    printFullState();
    stateBuffer();
    printFullState();
    while(get_state() != PARK_STATE){}
    printf("============================================\n\r");

    // Park to Reverse Drive
    printf("\n\rTesting: Park -> Reverse Drive==========\n\r");
    printf("ACTION: Switch to reverse gear\n\r");
    printFullState();
    stateBuffer();
    printFullState();
    while(get_state() != REVERSE_DRIVE){}
    printf("============================================\n\r");

    // Reverse Drive to Park
    printf("\n\rTesting: Park -> Reverse Drive==========\n\r");
    printf("ACTION: Switch to forward gear\n\r");
    printFullState();
    stateBuffer();
    printFullState();
    while(get_state() != PARK_STATE){}
    printf("ACTION: Switch to reverse gear\n\r");
    printFullState();
    stateBuffer();
    printFullState();
    while(get_state() != REVERSE_DRIVE){}
    printf("ACTION: Switch to park gear\n\r");
    printFullState();
    stateBuffer();
    printFullState();
    while(get_state() != PARK_STATE){}
    printf("============================================\n\r");


    OS_TaskSuspend(&SendTritium_TCB, &err);
    assertOSError(err);
    while (1){
        printf("\n\r\n\rSUCCESS! ALL TESTS PASSED\n\r\n\r");
        OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &err);
        assertOSError(err);
    }
};

int main()
{
    OS_ERR err;
    OSInit(&err);

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
    assertOSError(err);

    OSStart(&err);
}