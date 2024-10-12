#include "Minions.h"
#include "Pedals.h"
#include "CANbus.h"
#include "UpdateDisplay.h"
#include "ReadCarCAN.h"
#include "BSP_UART.h"
#include "Tasks.h"

#include "SendTritium.h"

#define MAX_VELOCITY 20000.0f  // rpm (unobtainable value)
#define MAX_GEARSWITCH_VELOCITY MpsToRpm(8.0f)  // rpm

#define BRAKE_PEDAL_THRESHOLD 50  // percent
#define ACCEL_PEDAL_THRESHOLD 10  // percent

#define PEDAL_MIN 0         // percent
#define PEDAL_MAX 100       // percent
#define CURRENT_SP_MIN 0    // percent
#define CURRENT_SP_MAX 100  // percent

static OS_TCB Task1TCB;
static CPU_STK Task1Stk[DEFAULT_STACK_SIZE];

void stateBuffer(){
    OS_ERR err;
    OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &err);
    assertOSError(err);
}

void gotoParkState() {
    SetCurrentSetpoint(0);
    SetVelocitySetpoint(MAX_VELOCITY);
    SetGear(kParkGear);
    stateBuffer();
}

void goToForwardDrive(){
    SetCurrentSetpoint(30);
    SetVelocitySetpoint(MAX_VELOCITY);
    SetGear(kForwardGear);
    stateBuffer();
}

void goToReverseDrive(){
    SetCurrentSetpoint(30);
    SetVelocitySetpoint(-MAX_VELOCITY);
    SetGear(kReverseGear);
    stateBuffer();
}

void Task1(void *arg)
{
    OS_ERR err;

    CPU_Init();
    BSP_UART_Init(UART_2);
    Pedals_Init();
    CANbus_Init(MOTORCAN, NULL, 0);
    Minions_Init();
    UpdateDisplay_Init();

    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U)OSCfg_TickRate_Hz);

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

    /**
     * ======= Forward Drive ==========
     * State Transitions: 
     * Brake, Record Velocity, One Pedal, Neutral, Reverse
    */
    printf("\n\r============ Testing Forward Drive State ============\n\r");

    // Forward Drive to Mechanical Brake
    printf("\n\rTesting: Forward Drive -> Mechanical Brake\n\r");
    goToForwardDrive();
    SetBrakePedalPercent(15);   // Simulate mechanical brake?
    stateBuffer();

    // Forward Drive to Park
    printf("\n\rTesting: Forward Drive -> Park\n\r");
    goToForwardDrive();
    SetBrakePedalPercent(15);
    gotoParkState();
    stateBuffer();

    // Forward Drive to Reverse (Should end up in park)
    printf("\n\rTesting: Forward Drive -> Reverse\n\r");
    goToForwardDrive();
    SetBrakePedalPercent(15);
    goToReverseDrive();
    stateBuffer();

    // Forward Drive to Reverse Drive (Should end up in Park)

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