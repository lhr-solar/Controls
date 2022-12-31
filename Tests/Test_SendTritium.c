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
extern float cruiseVelSetpoint;

static OS_TCB Task1TCB;
static CPU_STK Task1Stk[DEFAULT_STACK_SIZE];

void stateBuffer(){
    OS_ERR err;
    OSTimeDlyHMSM(0, 0, 0, 20, OS_OPT_TIME_HMSM_STRICT, &err);
    assertOSError(OS_UPDATE_VEL_LOC, err);
}

void goToBrakeState(){
    // Brake State
    brakePedalPercent = 15;
    stateBuffer();
}


void goToNormalState(){
    goToBrakeState();

    //Normal State
    brakePedalPercent = 0;
    stateBuffer();
}


void goToOnePedalDrive(){
    goToNormalState();

    // One Pedal Drive
    cruiseEnable = false;
    regenToggle = true;
    ChargeEnable = true;
    stateBuffer();
}

void goToRecordVelocity(){
    goToNormalState();

    // Record Velocity
    cruiseEnable = true;
    cruiseSet = true;
    stateBuffer();

}

void goToPoweredCruise(){
    goToRecordVelocity();

    // Powered Cruise
    cruiseEnable = true;
    cruiseSet = false;
    velocityObserved = 30;
    velocitySetpoint = 40;
    stateBuffer();
}

void goToCoastingCruise(){
    goToPoweredCruise();

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

    /**
     * ======= Powered Cruise ==========
     * State Transitions: 
     * Coasting Cruise, Normal State, Record Velocity, Brake State, Cruise Disable
    */
    printf("\n\r============ Testing Powered Cruise State ============\n\r");

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
    printf("\n\r============ Testing Coasting Cruise State ============\n\r");

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
    printf("\n\r============ Testing Normal State ============\n\r");

    // Normal Drive to Record Velocity
    goToNormalState();
    cruiseEnable = true;
    cruiseSet = true;
    brakePedalPercent = 0;
    stateBuffer();

    // Normal Drive to Brake State
    goToNormalState();
    brakePedalPercent = 15;
    stateBuffer();

    // Normal Drive to One Pedal Drive
    goToNormalState();
    cruiseEnable = false;
    regenToggle = true;
    ChargeEnable = true;
    stateBuffer();

   /**
     * ======= Brake State ==========
     * State Transitions: 
     * Brake Disable State
    */
    printf("\n\r============ Testing Brake State ============\n\r");


    // Brake State to One Pedal Drive
    goToBrakeState();
    brakePedalPercent = 0;
    stateBuffer();
   
   /**
     * ======= Record Velocity ==========
     * State Transitions: 
     * Brake State, Powered Cruise, Normal Drive
    */
    printf("\n\r============ Testing Record Velocity State ============\n\r");

    // Record Velocity to Brake State
    goToRecordVelocity();
    brakePedalPercent = 15;
    stateBuffer();

    // Record Velocity to Powered Cruise
    goToRecordVelocity();
    cruiseEnable = true;
    cruiseSet = false;
    stateBuffer();

    // Record Velocity to Normal Drive
    goToRecordVelocity();
    cruiseEnable = false;
    stateBuffer();

    /**
     * ======= One Pedal Drive ==========
     * State Transitions: 
     * One Pedal Disable, Brake State
    */
    printf("\n\r============ Testing One Pedal Drive State ============\n\r");

    // One Pedal Drive to One Pedal Disable
    goToOnePedalDrive();
    regenToggle = false;
    ChargeEnable = false;
    cruiseEnable = true;
    stateBuffer();

    // One Pedal Drive to Brake State
    goToOnePedalDrive();
    brakePedalPercent = 15;
    stateBuffer();

    /**
     * ======= Accelerate Cruise State ==========
     * State Transitions: 
     * Coasting Cruise, Brake State
    */
    printf("\n\r============ Testing Accelerate Cruise State ============\n\r");

    // Accelerate Cruise to Coasting Cruise
    goToCoastingCruise();
    accelPedalPercent = 0;
    stateBuffer();

    // Accelerate Cruise to Brake States
    goToOnePedalDrive();
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