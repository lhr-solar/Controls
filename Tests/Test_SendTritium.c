#include "Minions.h"
#include "Pedals.h"
#include "FaultState.h"
#include "CANbus.h"
#include "MotorController.h"
#include "ReadCarCAN.h"
#include "BSP_UART.h"
#include "Tasks.h"

// Test macro for SendTritium
#define __TEST_SENDTRITIUM
#include "SendTritium.h"

static OS_TCB Task1TCB;
static CPU_STK Task1Stk[DEFAULT_STACK_SIZE];

void stateBuffer(){
    OS_ERR err;
    OSTimeDlyHMSM(0, 0, 0, FSM_PERIOD + 10, OS_OPT_TIME_HMSM_STRICT, &err);
    assertOSError(OS_UPDATE_VEL_LOC, err);
}

void goToBrakeState(){
    // Brake State
    brakePedalPercent = 15;
    stateBuffer();
}


void goToForwardDrive(){
    goToBrakeState();

    //Forward Drive
    brakePedalPercent = 0;
    forwardGear = true;
    neutralGear = false;
    reverseGear = false;

    stateBuffer();
}

void goToNeutralDrive(){
    goToForwardDrive();

    // Neutral Drive
    forwardGear = false;
    neutralGear = true;
    reverseGear = false;

    stateBuffer();
}

void goToReverseDrive(){
    goToForwardDrive();
    
    // Reverse Drive
    forwardGear = false;
    neutralGear = false;
    reverseGear = true;

    stateBuffer();  // Transition to neutral

    velocityObserved = 5;
    stateBuffer();  // Transition to reverse
}


void goToOnePedalDrive(){
    goToForwardDrive();

    // One Pedal Drive
    cruiseEnable = false;
    onePedalEnable = true;
    chargeEnable = true;
    stateBuffer();
}

void goToRecordVelocity(){
    goToForwardDrive();

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
    cruiseVelSetpoint = 40;
    stateBuffer();
}

void goToCoastingCruise(){
    goToPoweredCruise();

    // Coasting Cruise
    cruiseEnable = true;
    cruiseSet = false;
    velocityObserved = 40;
    cruiseVelSetpoint = 30;
    stateBuffer();
}

void goToAccelerateCruise(){
    goToPoweredCruise();

    // Coasting Cruise
    cruiseEnable = true;
    cruiseSet = false;
    velocityObserved = 30;
    cruiseVelSetpoint = 30;
    accelPedalPercent = 10;
    stateBuffer();
}

void Task1(void *arg)
{
    OS_ERR err;

    CPU_Init();
    BSP_UART_Init(UART_2);
    Pedals_Init();
    CANbus_Init(MOTORCAN);
    Minion_Init();

    OS_CPU_SysTickInit(SystemCoreClock / (CPU_INT32U)OSCfg_TickRate_Hz);
    chargeEnable = ON;

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
     * ======= Forward Drive ==========
     * State Transitions: 
     * Brake, Record Velocity, One Pedal, Neutral Drive
    */
    printf("\n\r============ Testing Forward Drive State ============\n\r");

    // Forward Drive to Brake
    printf("\n\rTesting: Forward Drive -> Brake\n\r");
    goToForwardDrive();
    brakePedalPercent = 15;
    stateBuffer();

    // Forward Drive to Record Velocity
    printf("\n\rTesting: Forward Drive -> Record Velocity\n\r");
    goToForwardDrive();
    cruiseEnable = true;
    cruiseSet = true;
    stateBuffer();

    // Forward Drive to One Pedal
    printf("\n\rTesting: Forward Drive -> One Pedal\n\r");
    goToForwardDrive();
    onePedalEnable = true;
    stateBuffer();

    // Forward Drive to Neutral Drive
    printf("\n\rTesting: Forward Drive -> Neutral Drive\n\r");
    goToForwardDrive();
    forwardGear = false;
    neutralGear = true;
    reverseGear = false;

    // Forward Drive to Reverse Drive
    printf("\n\rTesting: Reverse Drive -> Forward Drive\n\r");
    goToReverseDrive();
    forwardGear = false;
    neutralGear = false;
    reverseGear = true;
    velocityObserved = 35;
    stateBuffer();
    velocityObserved = 5;
    stateBuffer();

    /**
     * ======= Neutral Drive ==========
     * State Transitions: 
     * Brake, Forward Drive, Reverse Drive
    */

    printf("\n\r============ Testing Neutral Drive State ============\n\r");

    // Neutral Drive to Brake
    printf("\n\rTesting: Neutral Drive -> Brake\n\r");
    goToNeutralDrive();
    brakePedalPercent = 15;
    stateBuffer();

    // Neutral Drive to Forward Drive
    printf("\n\rTesting: Neutral Drive -> Forward Drive\n\r");
    goToNeutralDrive();
    velocityObserved = 5;
    forwardGear = true;
    neutralGear = false;
    reverseGear = false;
    stateBuffer();

    // Neutral Drive to Reverse Drive
    printf("\n\rTesting: Neutral Drive -> Reverse Drive\n\r");
    goToNeutralDrive();
    velocityObserved = 5;
    forwardGear = false;
    neutralGear = false;
    reverseGear = true;
    stateBuffer();

    /**
     * ======= Reverse Drive ==========
     * State Transitions: 
     * Brake, Neutral Drive
    */

    printf("\n\r============ Testing Reverse Drive State ============\n\r");
    
    // Reverse Drive to Brake
    printf("\n\rTesting: Reverse Drive -> Brake\n\r");
    goToReverseDrive();
    brakePedalPercent = 15;
    stateBuffer();

    // Reverse Drive to Neutral Drive
    printf("\n\rTesting: Reverse Drive -> Neutral Drive\n\r");
    goToReverseDrive();
    forwardGear = false;
    neutralGear = true;
    reverseGear = false;
    stateBuffer();

    // Reverse Drive to Forward Drive
    printf("\n\rTesting: Reverse Drive -> Forward Drive\n\r");
    goToReverseDrive();
    forwardGear = true;
    neutralGear = false;
    reverseGear = false;
    velocityObserved = 35;
    stateBuffer();
    velocityObserved = 5;
    stateBuffer();

    /**
     * ======= One Pedal Drive ==========
     * State Transitions: 
     * Brake, Neutral Drive, Forward Drive
    */

    printf("\n\r============ Testing One Pedal Drive State ============\n\r");

    // One Pedal Drive to Brake
    printf("\n\rTesting: One Pedal Drive -> Brake\n\r");
    goToOnePedalDrive();
    brakePedalPercent = 15;
    stateBuffer();

    // One Pedal Drive to Neutral Drive
    printf("\n\rTesting: One Pedal Drive -> Neutral Drive\n\r");
    goToOnePedalDrive();
    forwardGear = false;
    neutralGear = true;
    reverseGear = false;
    stateBuffer();

    // One Pedal Drive to Forward Drive
    printf("\n\rTesting: One Pedal Drive -> Forward Drive\n\r");
    goToOnePedalDrive();
    forwardGear = true;
    neutralGear = false;
    reverseGear = false;
    stateBuffer();

    /**
     * ======= Powered Cruise ==========
     * State Transitions: 
     * Brake, Neutral, One Pedal, Forward Drive, Record Velocity, Accelerate Cruise, Coasting Cruise
    */
    printf("\n\r============ Testing Powered Cruise State ============\n\r");

    // Powered Cruise to Brake State
    printf("\n\rTesting: Powered Cruise -> Brake\n\r");
    goToPoweredCruise();
    brakePedalPercent = 15;
    stateBuffer();

    // Powered Cruise to Neutral Drive
    printf("\n\rTesting: Powered Cruise -> Neutral Drive\n\r");
    goToPoweredCruise();
    forwardGear = false;
    neutralGear = true;
    reverseGear = false;
    stateBuffer();

    // Powered Cruise to One Pedal Drive
    printf("\n\rTesting: Powered Cruise -> One Pedal Drive\n\r");
    goToPoweredCruise();
    onePedalEnable = true;
    stateBuffer();

    // Powered Cruise to Forward Drive
    printf("\n\rTesting: Powered Cruise -> Forward Drive\n\r");
    goToPoweredCruise();
    cruiseEnable = false;
    stateBuffer();

    // Powered Cruise to Record Velocity
    printf("\n\rTesting: Powered Cruise -> Record Velocity\n\r");
    goToPoweredCruise();
    cruiseSet = true;
    cruiseEnable = true;
    stateBuffer();

    // Powered Cruise to Accelerate Cruise
    printf("\n\rTesting: Powered Cruise -> Accelerate Cruise\n\r");
    goToPoweredCruise();
    accelPedalPercent = 10;
    stateBuffer();

    // Powered Cruise to Coasting Cruise
    printf("\n\rTesting: Powered Cruise -> Coasting Cruise\n\r");
    goToPoweredCruise();
    velocityObserved = 40;
    cruiseVelSetpoint = 30;
    stateBuffer();

    /**
     * ======= Coasting Cruise ==========
     * State Transitions: 
     * Brake, Neutral Drive, One Pedal, Forward Drive, Record Velocity, 
     * Accelerate Cruise, Powered Cruise
    */
    printf("\n\r============ Testing Coasting Cruise State ============\n\r");

    // Coasting Cruise to Brake State
    printf("\n\rTesting: Coasting Cruise -> Brake\n\r");
    goToCoastingCruise();
    brakePedalPercent = 15;
    stateBuffer();

    // Coasting Cruise to Neutral Drive
    printf("\n\rTesting: Coasting Cruise -> Neutral Drive\n\r");
    goToCoastingCruise();
    forwardGear = false;
    neutralGear = true;
    reverseGear = false;
    stateBuffer();

    // Coasting Cruise to One Pedal Drive
    printf("\n\rTesting: Coasting Cruise -> One Pedal Drive\n\r");
    goToCoastingCruise();
    onePedalEnable = true;
    stateBuffer();

    // Coasting Cruise to Forward Drive
    printf("\n\rTesting: Coasting Cruise -> Forward Drive\n\r");
    goToCoastingCruise();
    cruiseEnable = false;
    stateBuffer();

    // Coasting Cruise to Record Velocity
    printf("\n\rTesting: Coasting Cruise -> Record Velocity\n\r");
    goToCoastingCruise();
    cruiseSet = true;
    cruiseEnable = true;
    stateBuffer();

    // Coasting Cruise to Accelerate Cruise
    printf("\n\rTesting: Coasting Cruise -> Accelerate Cruise\n\r");
    goToCoastingCruise();
    accelPedalPercent = 10;
    stateBuffer();

    // Coasting Cruise to Powered Cruise
    printf("\n\rTesting: Powered Cruise -> Coasting Cruise\n\r");
    goToPoweredCruise();
    velocityObserved = 29;
    cruiseVelSetpoint = 30;
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
    onePedalEnable = false;
    chargeEnable = false;
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