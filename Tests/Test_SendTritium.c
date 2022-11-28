#include "Minions.h"
#include "Pedals.h"
#include "FaultState.h"
#include "CANbus.h"
#include "MotorController.h"
#include "ReadCarCAN.h"
#include "BSP_UART.h"
#include "Tasks.h"
#include "SendTritium.h"

// Inputs
extern bool cruiseEnable;
extern bool cruiseSet;
extern bool regenToggle;
extern uint8_t brakePedalPercent;
extern uint8_t accelPedalPercent;
extern bool reverseSwitch;
extern bool forwardSwitch;
extern uint8_t state;
extern float currentSetpoint;
extern float velocitySetpoint;
extern float velocityObserved;

static OS_TCB Task1TCB;
static CPU_STK Task1Stk[DEFAULT_STACK_SIZE];

void stateBuffer(){
    OS_ERR err;
    OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &err);
    assertOSError(OS_UPDATE_VEL_LOC, err);
}


void goToNormalState(){
    printf("\n\rGoing to Normal State\n\r");

    // Brake State
    brakePedalPercent = 15;
    stateBuffer();

    // Disable One Pedal --> Normal State
    brakePedalPercent = 0;
    stateBuffer();
}

void goToPoweredCruise(){
    printf("\n\rGoing to Powered Cruise State\n\r");

    goToNormalState();

    // Record Velocity
    cruiseEnable = true;
    cruiseSet = true;
    stateBuffer();

    // Powered Cruise
    cruiseEnable = true;
    cruiseSet = false;
    velocityObserved = 30;
    velocitySetpoint = 40;
    stateBuffer();
}

void goToCoastingCruise(){
    printf("\n\rGoing to Coasting Cruise State\n\r");

    goToNormalState();

    // Record Velocity
    cruiseEnable = true;
    cruiseSet = true;
    stateBuffer();

    // Powered Cruise
    cruiseEnable = true;
    cruiseSet = false;
    velocityObserved = 30;
    velocitySetpoint = 40;
    stateBuffer();

    // Coasting Cruise
    cruiseEnable = true;
    cruiseSet = false;
    velocityObserved = 40;
    velocitySetpoint = 30;
    stateBuffer();

}

void Task1(void *arg)
{
    OS_ERR err;

    CPU_Init();
    BSP_UART_Init(UART_2);
    Pedals_Init();
    //OSTimeDlyHMSM(0,0,5,0,OS_OPT_TIME_HMSM_STRICT,&err);
    MotorController_Init(1.0f); // Let motor controller use 100% of bus current
    //OSTimeDlyHMSM(0,0,10,0,OS_OPT_TIME_HMSM_STRICT,&err);
    CANbus_Init();
    Minion_Init();

    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U)OSCfg_TickRate_Hz);
    ChargeEnable = ON;

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
    //assertOSError(err);

    // Normal Drive
    cruiseEnable = false;
    cruiseSet = false;
    regenToggle = false;
    brakePedalPercent = 0;
    accelPedalPercent = 0;
    reverseSwitch = false;
    forwardSwitch = false;
    velocityObserved = 0;
    stateBuffer();

    // Record Velocity
    cruiseEnable = true;
    cruiseSet = true;
    stateBuffer();

    // Powered Cruise
    cruiseEnable = true;
    cruiseSet = false;
    stateBuffer();

    // Coasting Cruise
    cruiseEnable = true;
    cruiseSet = false;
    velocityObserved = 40;
    velocitySetpoint = 30;
    stateBuffer();

    // Powered Cruise
    cruiseEnable = true;
    cruiseSet = false;
    velocityObserved = 30;
    velocitySetpoint = 40;
    stateBuffer();

    // Brake State
    brakePedalPercent = 15;
    stateBuffer();

    // Normal Drive
    brakePedalPercent = 0;
    stateBuffer();

    // One Pedal Drive
    cruiseEnable = false;
    regenToggle = true;
    ChargeEnable = true;
    stateBuffer();

    /**
     * ======= Powered Cruise ==========
     * State Transitions: 
     * Coasting Cruise, Normal State, Record Velocity, Brake State, Cruise Disable
    */

    // Powered Cruise to Coasting Cruise
    goToPoweredCruise();
    velocityObserved = 40;
    velocitySetpoint = 30;
    stateBuffer();

    // Powered Cruise to Normal Drive
    goToPoweredCruise();
    cruiseEnable = false;
    stateBuffer();

    // Powered Cruise to Record Velocity
    goToPoweredCruise();
    cruiseSet = true;
    cruiseEnable = true;
    stateBuffer();

    // Powered Cruise to Brake State
    goToPoweredCruise();
    brakePedalPercent = 15;
    stateBuffer();

    // Powered Cruise to Cruise Disable
    goToPoweredCruise();
    regenToggle = true;
    stateBuffer();

    /**
     * ======= Coasting Cruise ==========
     * State Transitions: 
     * Powered Cruise, Normal Drive, Brake State, Cruise Disable
    */

    // Coasting Cruise to Powered Cruise
    goToCoastingCruise();
    velocitySetpoint = 30;
    velocityObserved = 40;
    stateBuffer();

    // Coasting Cruise to Normal State
    goToCoastingCruise();
    cruiseEnable = false;
    stateBuffer();

    // Coasting Cruise to Brake State
    goToCoastingCruise();
    brakePedalPercent = 15;
    stateBuffer();

    // Coasting Cruise to Cruise Disable -> Normal State
    goToCoastingCruise();
    brakePedalPercent = 15;
    stateBuffer();

    /**
     * ======= Normal Drive ==========
     * State Transitions: 
     * Record Velocity, One Pedal Drive, Brake State
    */

   // Normal Drive to Record Velocity
   goToNormalState();
   cruiseEnable = true;
   cruiseSet = true;
   brakePedalPercent = 0;
   stateBuffer();

   // Normal Drive to One Pedal Drive
   goToNormalState();
   cruiseEnable = false;
   regenToggle = true;
   ChargeEnable = true;
   stateBuffer();

   // Normal Drive to Brake State
   goToNormalState();
   brakePedalPercent = 15;
   stateBuffer();





    while (1){
        OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &err);
        assertOSError(OS_MAIN_LOC, err);
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