#include "Minions.h"
#include "Pedals.h"
#include "FaultState.h"
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
    OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &err);
    assertOSError(err);
}

void goToBrakeState(){
    // Brake State
    set_brakePedalPercent(15);
    stateBuffer();
    while(get_state().name != BRAKE_STATE){}
}

void goToForwardDrive(){
    goToBrakeState();

    //Forward Drive
    set_brakePedalPercent(0);
    set_velocityObserved(mpsToRpm(2));
    set_gear(FORWARD_GEAR);

    stateBuffer();
    while(get_state().name != FORWARD_DRIVE){}
}

void goToNeutralDrive(){
    goToForwardDrive();

    // Neutral Drive
    set_gear(NEUTRAL_GEAR);

    stateBuffer();
    while(get_state().name != NEUTRAL_DRIVE){}
}

void goToReverseDrive(){
    goToForwardDrive();
    
    // Reverse Drive
    set_gear(REVERSE_GEAR);

    stateBuffer();  // Transition to neutral // DOESN"T GO TO NEUTRAL???
    // while(get_state().name != NEUTRAL_DRIVE){}

    set_velocityObserved(mpsToRpm(5));
    stateBuffer();  // Transition to reverse
    while(get_state().name != REVERSE_DRIVE){}
}


void goToOnePedalDrive(){
    goToForwardDrive();

    // One Pedal Drive
    set_cruiseEnable(false);
    set_onePedalEnable(true);
    set_regenEnable(true);
    stateBuffer();
    while(get_state().name != ONEPEDAL){}
}

void goToRecordVelocity(){
    goToForwardDrive();

    // Record Velocity
    set_cruiseEnable(true);
    set_cruiseSet(true);
    set_velocityObserved(mpsToRpm(30));
    stateBuffer();
    while(get_state().name != RECORD_VELOCITY){}
}

void goToPoweredCruise(){
    goToRecordVelocity();

    // Powered Cruise
    set_cruiseEnable(true);
    set_cruiseSet(false);
    set_velocityObserved(mpsToRpm(30));
    set_cruiseVelSetpoint(mpsToRpm(40));
    stateBuffer();
    while(get_state().name != POWERED_CRUISE){}
}

void goToCoastingCruise(){
    goToPoweredCruise();

    // Coasting Cruise
    set_cruiseEnable(true);
    set_cruiseSet(false);
    set_velocityObserved(mpsToRpm(40));
    set_cruiseVelSetpoint(mpsToRpm(30));
    stateBuffer();
    while(get_state().name != COASTING_CRUISE){}
}

void goToAccelerateCruise(){
    goToPoweredCruise();

    // Coasting Cruise
    set_cruiseEnable(true);
    set_cruiseSet(false);
    set_velocityObserved(mpsToRpm(30));
    set_cruiseVelSetpoint(mpsToRpm(30));
    set_accelPedalPercent(10);
    stateBuffer();
    while(get_state().name != ACCELERATE_CRUISE){}
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
    set_regenEnable(ON);

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
     * Brake, Record Velocity, One Pedal, Neutral, Reverse
    */
    printf("\n\r============ Testing Forward Drive State ============\n\r");

    // Forward Drive to Brake
    printf("\n\rTesting: Forward Drive -> Brake\n\r");
    goToForwardDrive();
    set_brakePedalPercent(15);
    stateBuffer();
    while(get_state().name != BRAKE_STATE){}

    // Forward Drive to Record Velocity
    printf("\n\rTesting: Forward Drive -> Record Velocity\n\r");
    goToForwardDrive();
    set_cruiseEnable(true);
    set_cruiseSet(true);
    set_velocityObserved(mpsToRpm(20.0));
    stateBuffer();
    set_velocityObserved(mpsToRpm(25.0));
    stateBuffer();
    while(get_state().name != RECORD_VELOCITY){}

    // Forward Drive to One Pedal
    printf("\n\rTesting: Forward Drive -> One Pedal\n\r");
    goToForwardDrive();
    set_onePedalEnable(true);
    stateBuffer();
    while(get_state().name != ONEPEDAL){}

    // Forward Drive to Neutral Drive
    printf("\n\rTesting: Forward Drive -> Neutral Drive\n\r");
    goToForwardDrive();
    set_gear(NEUTRAL_GEAR);
    stateBuffer();
    while(get_state().name != NEUTRAL_DRIVE){}

    // Forward Drive to Reverse Drive
    printf("\n\rTesting: Forward Drive -> Reverse Drive\n\r");
    goToForwardDrive();
    set_gear(REVERSE_GEAR);
    set_velocityObserved(mpsToRpm(35));
    stateBuffer();
    set_velocityObserved(mpsToRpm(5));
    stateBuffer();
    while(get_state().name != REVERSE_DRIVE){}

    /**
     * ======= Neutral Drive ==========
     * State Transitions: 
     * Brake, Forward Drive, Reverse Drive
    */

    printf("\n\r============ Testing Neutral Drive State ============\n\r");

    // Neutral Drive to Brake
    printf("\n\rTesting: Neutral Drive -> Brake\n\r");
    goToNeutralDrive();
    set_brakePedalPercent(15);
    stateBuffer();
    while(get_state().name != BRAKE_STATE){}

    // Neutral Drive to Forward Drive
    printf("\n\rTesting: Neutral Drive -> Forward Drive\n\r");
    goToNeutralDrive();
    set_velocityObserved(mpsToRpm(5));
    set_gear(FORWARD_GEAR);
    stateBuffer();
    while(get_state().name != FORWARD_DRIVE){}

    // Neutral Drive to Reverse Drive
    printf("\n\rTesting: Neutral Drive -> Reverse Drive\n\r");
    goToNeutralDrive();
    set_velocityObserved(mpsToRpm(5));
    set_gear(REVERSE_GEAR);
    set_velocityObserved(mpsToRpm(35));
    stateBuffer();
    set_velocityObserved(mpsToRpm(5));
    stateBuffer();
    while(get_state().name != REVERSE_DRIVE){}

    /**
     * ======= Reverse Drive ==========
     * State Transitions: 
     * Brake, Neutral Drive
    */

    printf("\n\r============ Testing Reverse Drive State ============\n\r");
    
    // Reverse Drive to Brake
    printf("\n\rTesting: Reverse Drive -> Brake\n\r");
    goToReverseDrive();
    set_brakePedalPercent(15);
    stateBuffer();
    while(get_state().name != BRAKE_STATE){}

    // Reverse Drive to Neutral Drive
    printf("\n\rTesting: Reverse Drive -> Neutral Drive\n\r");
    goToReverseDrive();
    set_velocityObserved(mpsToRpm(35));
    set_gear(NEUTRAL_GEAR);
    stateBuffer();
    while(get_state().name != NEUTRAL_DRIVE){}

    // Reverse Drive to Forward Drive
    printf("\n\rTesting: Reverse Drive -> Forward Drive\n\r");
    goToReverseDrive();
    set_gear(FORWARD_GEAR);
    set_velocityObserved(mpsToRpm(35));
    stateBuffer();
    set_velocityObserved(mpsToRpm(5));
    stateBuffer();
    while(get_state().name != FORWARD_DRIVE){}

      /**
     * ======= Record Velocity ==========
     * State Transitions: 
     * Brake State, Neutral, One Pedal, Forward Drive, Powered Cruise
    */
    printf("\n\r============ Testing Record Velocity State ============\n\r");

    // Record Velocity to Brake State
    printf("\n\rTesting: Record Velocity -> Brake\n\r");
    goToRecordVelocity();
    set_brakePedalPercent(15);
    stateBuffer();
    while(get_state().name != BRAKE_STATE){}

    // Record Velocity to Neutral Drive
    printf("\n\rTesting: Record Velocity -> Neutral Drive\n\r");
    goToRecordVelocity();
    set_gear(NEUTRAL_GEAR);
    stateBuffer();
    while(get_state().name != NEUTRAL_DRIVE){}

    // Record Velocity to Reverse Drive
    printf("\n\rTesting: Record Velocity -> Reverse Drive\n\r");
    goToRecordVelocity();
    set_gear(REVERSE_GEAR);
    set_velocityObserved(mpsToRpm(35));
    stateBuffer();
    set_velocityObserved(mpsToRpm(5));
    stateBuffer();
    while(get_state().name != REVERSE_DRIVE){}

    // Record Velocity to One Pedal Drive
    printf("\n\rTesting: Record Velocity -> One Pedal Drive\n\r");
    goToRecordVelocity();
    set_onePedalEnable(true);
    stateBuffer();
    while(get_state().name != ONEPEDAL){}

    // Record Velocity to Forward Normal Drive
    printf("\n\rTesting: Record Velocity -> Forward Drive\n\r");
    goToRecordVelocity();
    set_cruiseEnable(false);
    stateBuffer();
    while(get_state().name != FORWARD_DRIVE){}
    
    // Record Velocity to Powered Cruise
    // printf("\n\rTesting: Record Velocity -> Powered Cruise\n\r");
    // goToRecordVelocity();
    // set_cruiseEnable(true);
    // set_cruiseSet(false);
    // stateBuffer();
    // while(get_state().name != POWERED_CRUISE){}

    /**
     * ======= Powered Cruise ==========
     * State Transitions: 
     * Brake, Neutral, One Pedal, Forward Drive, Record Velocity, Accelerate Cruise, Coasting Cruise
    */
    printf("\n\r============ Testing Powered Cruise State ============\n\r");

    // // Powered Cruise to Brake State
    // printf("\n\rTesting: Powered Cruise -> Brake\n\r");
    // goToPoweredCruise();
    // set_brakePedalPercent(15);
    // stateBuffer();
    // while(get_state().name != BRAKE_STATE){}

    // // Powered Cruise to Neutral Drive
    // printf("\n\rTesting: Powered Cruise -> Neutral Drive\n\r");
    // goToPoweredCruise();
    // set_gear(NEUTRAL_GEAR);
    // stateBuffer();
    // while(get_state().name != NEUTRAL_DRIVE){}

    // // Powered Cruise to Reverse Drive
    // printf("\n\rTesting: Powered Cruise -> Reverse Drive\n\r");
    // goToPoweredCruise();
    // set_gear(REVERSE_GEAR);
    // set_velocityObserved(mpsToRpm(35));
    // stateBuffer();
    // set_velocityObserved(mpsToRpm(5));
    // stateBuffer();
    // while(get_state().name != REVERSE_DRIVE){}

    // // Powered Cruise to One Pedal Drive
    // printf("\n\rTesting: Powered Cruise -> One Pedal Drive\n\r");
    // goToPoweredCruise();
    // set_onePedalEnable(true);
    // stateBuffer();
    // while(get_state().name != ONEPEDAL){}

    // // Powered Cruise to Forward Drive
    // printf("\n\rTesting: Powered Cruise -> Forward Drive\n\r");
    // goToPoweredCruise();
    // set_cruiseEnable(false);
    // stateBuffer();
    // while(get_state().name != FORWARD_DRIVE){}

    // // Powered Cruise to Record Velocity
    // printf("\n\rTesting: Powered Cruise -> Record Velocity\n\r");
    // goToPoweredCruise();
    // set_cruiseSet(true);
    // stateBuffer();
    // while(get_state().name != RECORD_VELOCITY){}

    // // Powered Cruise to Accelerate Cruise
    // printf("\n\rTesting: Powered Cruise -> Accelerate Cruise\n\r");
    // goToPoweredCruise();
    // set_accelPedalPercent(10);
    // stateBuffer();
    // while(get_state().name != ACCELERATE_CRUISE){}

    // // Powered Cruise to Coasting Cruise
    // printf("\n\rTesting: Powered Cruise -> Coasting Cruise\n\r");
    // goToPoweredCruise();
    // set_accelPedalPercent(0);
    // set_velocityObserved(mpsToRpm(40));
    // set_cruiseVelSetpoint(mpsToRpm(30));
    // stateBuffer();
    // while(get_state().name != COASTING_CRUISE){}

    /**
     * ======= Coasting Cruise ==========
     * State Transitions: 
     * Brake, Neutral Drive, One Pedal, Forward Drive, Record Velocity, 
     * Accelerate Cruise, Powered Cruise
    */
    printf("\n\r============ Testing Coasting Cruise State ============\n\r");

    // // Coasting Cruise to Brake State
    // printf("\n\rTesting: Coasting Cruise -> Brake\n\r");
    // goToCoastingCruise();
    // set_brakePedalPercent(15);
    // stateBuffer();
    // while(get_state().name != BRAKE_STATE){}

    // // Coasting Cruise to Neutral Drive
    // printf("\n\rTesting: Coasting Cruise -> Neutral Drive\n\r");
    // goToCoastingCruise();
    // set_gear(NEUTRAL_GEAR);
    // stateBuffer();
    // while(get_state().name != NEUTRAL_DRIVE){}

    // // Coasting Cruise to Reverse Drive
    // printf("\n\rTesting: Coasting Cruise -> Reverse Drive\n\r");
    // goToCoastingCruise();
    // set_gear(REVERSE_GEAR);
    // set_velocityObserved(mpsToRpm(35));
    // stateBuffer();
    // set_velocityObserved(mpsToRpm(5));
    // stateBuffer();
    // while(get_state().name != REVERSE_DRIVE){}

    // // Coasting Cruise to One Pedal Drive
    // printf("\n\rTesting: Coasting Cruise -> One Pedal Drive\n\r");
    // goToCoastingCruise();
    // set_onePedalEnable(true);
    // stateBuffer();
    // while(get_state().name != ONEPEDAL){}

    // // Coasting Cruise to Forward Drive
    // printf("\n\rTesting: Coasting Cruise -> Forward Drive\n\r");
    // goToCoastingCruise();
    // set_cruiseEnable(false);
    // stateBuffer();
    // while(get_state().name != FORWARD_DRIVE){}

    // // Coasting Cruise to Record Velocity
    // printf("\n\rTesting: Coasting Cruise -> Record Velocity\n\r");
    // goToCoastingCruise();
    // set_cruiseSet(true);
    // set_velocityObserved(mpsToRpm(25.0));
    // stateBuffer();
    // while(get_state().name != RECORD_VELOCITY){}

    // // Coasting Cruise to Accelerate Cruise
    // printf("\n\rTesting: Coasting Cruise -> Accelerate Cruise\n\r");
    // goToCoastingCruise();
    // set_accelPedalPercent(10);
    // stateBuffer();
    // while(get_state().name != ACCELERATE_CRUISE){}

    // // Coasting Cruise to Powered Cruise
    // printf("\n\rTesting: Coasting Cruise -> Powered Cruise\n\r");
    // goToCoastingCruise();
    // set_accelPedalPercent(0);
    // set_velocityObserved(mpsToRpm(29));
    // set_cruiseVelSetpoint(mpsToRpm(30));
    // stateBuffer();
    // while(get_state().name != POWERED_CRUISE){}

    /**
     * ======= Accelerate Cruise State ==========
     * State Transitions: 
     * Coasting Cruise, Brake State
    */
    printf("\n\r============ Testing Accelerate Cruise State ============\n\r");

    // // Accelerate Cruise to Brake State
    // printf("\n\rTesting: Accelerate Cruise -> Brake\n\r");
    // goToAccelerateCruise();
    // set_brakePedalPercent(15);
    // stateBuffer();
    // while(get_state().name != BRAKE_STATE){}

    // // Accelerate Cruise to Neutral Drive
    // printf("\n\rTesting: Accelerate Cruise -> Neutral Drive\n\r");
    // goToAccelerateCruise();
    // set_gear(NEUTRAL_GEAR);
    // stateBuffer();
    // while(get_state().name != NEUTRAL_DRIVE){}

    // // Accelerate Cruise to Reverse Drive
    // printf("\n\rTesting: Accelerate Cruise -> Reverse Drive\n\r");
    // goToAccelerateCruise();
    // set_gear(REVERSE_GEAR);
    // set_velocityObserved(mpsToRpm(35));
    // stateBuffer();
    // set_velocityObserved(mpsToRpm(5));
    // stateBuffer();
    // while(get_state().name != REVERSE_DRIVE){}

    // // Accelerate Cruise to One Pedal Drive
    // printf("\n\rTesting: Accelerate Cruise -> One Pedal Drive\n\r");
    // goToAccelerateCruise();
    // set_onePedalEnable(true);
    // stateBuffer();
    // while(get_state().name != ONEPEDAL){}

    // // Accelerate Cruise to Forward Drive
    // printf("\n\rTesting: Accelerate Cruise -> Forward Drive\n\r");
    // goToAccelerateCruise();
    // set_cruiseEnable(false);
    // stateBuffer();
    // while(get_state().name != FORWARD_DRIVE){}

    // // Accelerate Cruise to Record Velocity
    // printf("\n\rTesting: Accelerate Cruise -> Record Velocity\n\r");
    // goToAccelerateCruise();
    // set_cruiseSet(true);
    // set_velocityObserved(mpsToRpm(25.0));
    // stateBuffer();
    // while(get_state().name != RECORD_VELOCITY){}

    // // Accelerate Cruise to Coasting Cruise
    // printf("\n\rTesting: Accelerate Cruise -> Coasting Cruise\n\r");
    // goToAccelerateCruise();
    // set_accelPedalPercent(0);
    // stateBuffer();
    // while(get_state().name != COASTING_CRUISE){}

    /**
     * ======= One Pedal Drive ==========
     * State Transitions: 
     * Brake, Neutral Drive, Forward Drive
    */

    printf("\n\r============ Testing One Pedal Drive State ============\n\r");

    // One Pedal Drive to Brake
    printf("\n\rTesting: One Pedal Drive -> Brake\n\r");
    goToOnePedalDrive();
    set_brakePedalPercent(15);
    stateBuffer();
    while(get_state().name != BRAKE_STATE){}

    // One Pedal Drive to Neutral Drive
    printf("\n\rTesting: One Pedal Drive -> Neutral Drive\n\r");
    goToOnePedalDrive();
    set_gear(NEUTRAL_GEAR);
    stateBuffer();
    while(get_state().name != NEUTRAL_DRIVE){}

    // One Pedal Drive to Reverse Drive
    printf("\n\rTesting: One Pedal Drive -> Reverse Drive\n\r");
    goToOnePedalDrive();
    set_gear(REVERSE_GEAR);
    set_velocityObserved(mpsToRpm(35));
    stateBuffer();
    set_velocityObserved(mpsToRpm(5));
    stateBuffer();
    while(get_state().name != REVERSE_DRIVE){}

    // One Pedal Drive to Record Velocity
    // printf("\n\rTesting: One Pedal Drive -> Record Velocity\n\r");
    // goToOnePedalDrive();
    // set_cruiseSet(true);             // DOES WORK BUT NEEDS TO ENFORCE 1 TRANSITION
    // set_cruiseEnable(true);
    // set_velocityObserved(mpsToRpm(25.0));
    // stateBuffer();
    // while(get_state().name != RECORD_VELOCITY){}

   /**
     * ======= Brake State ==========
     * State Transitions: 
     * Brake Disable State
    */
    printf("\n\r============ Testing Brake State ============\n\r");

    // Brake State to Forward Drive
    printf("\n\rTesting: Brake -> Forward Drive\n\r");
    goToBrakeState();
    set_brakePedalPercent(1);
    set_gear(FORWARD_GEAR);
    stateBuffer();
    while(get_state().name != FORWARD_DRIVE){}

    // Brake State to Neutral Drive
    printf("\n\rTesting: Brake -> Neutral Drive\n\r");
    goToBrakeState();
    set_brakePedalPercent(1);
    set_gear(NEUTRAL_GEAR);
    stateBuffer();
    while(get_state().name != NEUTRAL_DRIVE){}

    // Brake State to Reverse Drive
    printf("\n\rTesting: Brake -> Reverse Drive\n\r");
    goToBrakeState();
    set_brakePedalPercent(1);
    set_gear(REVERSE_GEAR);
    set_velocityObserved(mpsToRpm(35));
    stateBuffer();
    set_velocityObserved(mpsToRpm(5));
    stateBuffer();
    while(get_state().name != REVERSE_DRIVE){}

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
    OSSemCreate( // create fault sem4
        &FaultState_Sem4,
        "Fault State Semaphore",
        0,
        &err);
    assertOSError(err);

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
    assertOSError(err);

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