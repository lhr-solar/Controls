#include "Minions.h"
#include "Pedals.h"
#include "FaultState.h"
#include "CANbus.h"
#include "UpdateDisplay.h"
#include "ReadCarCAN.h"
#include "BSP_UART.h"
#include "Tasks.h"

#include "SendTritium.h"

// Macros
#define VEL_EXCEED_CRUISE_THRESH (MIN_CRUISE_VELOCITY + 2.0f)
#define VEL_BELOW_CRUISE_THRESH 0.0f

#define ACCEL_UNPRESSED 0
#define ACCEL_PRESSED 100

// SendTritium.h has BRAKE_UNPRESSED & BRAKE_PRESSED already defined

static OS_TCB Task1TCB;
static CPU_STK Task1Stk[DEFAULT_STACK_SIZE];

void stateBuffer(){
    OS_ERR err;
    OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &err);
    assertOSError(err);
}

/**
 * ======= Testing Utilities & Variables ==========
 * Wrappers to set velocity, accel, & brake according to enum parameters &
 * to call state deciders/switch states
*/
float cruiseVelTestVar = 0.0f;

void stateHandler() {
    TritiumStateName_t stateName = get_state();
    switch (stateName) {
        case FORWARD_DRIVE: ForwardDriveHandler();
        case PARK_STATE: ParkHandler();
        case REVERSE_DRIVE: ReverseDriveHandler();
        case POWERED_CRUISE: PoweredCruiseHandler();
        case COASTING_CRUISE: CoastingCruiseHandler();
        case ACCELERATE_CRUISE: AccelerateCruiseHandler();
    }        
}

void stateDecider() {
    TritiumStateName_t stateName = get_state();
    switch (stateName) {
        case FORWARD_DRIVE: ForwardDriveDecider();
        case PARK_STATE: ParkDecider();
        case REVERSE_DRIVE: ReverseDriveDecider();
        case POWERED_CRUISE: PoweredCruiseDecider();
        case COASTING_CRUISE: CoastingCruiseDecider();
        case ACCELERATE_CRUISE: AccelerateCruiseDecider();
    }    
}

/**
 * ======= State Setters ==========
 * States:
 * Forward Drive, Park State, Reverse Drive,
 * PoweredCruise, CoastingCruise, AccelerateCruise
*/
void goToForwardDrive(float velocityObserved){
    // Set forward drive state & gear
    set_state(FORWARD_DRIVE);
    set_gear(FORWARD_GEAR);

    // Set cruise buttons
    set_cruiseEnable(false);
    set_cruiseSet(false);

    // Set velocityObserved
    set_velocityObserved(velocityObserved);
}

void goToParkState(float velocityObserved){
    // Set park state & gear
    set_state(PARK_STATE);
    set_gear(PARK_GEAR);

    // Set cruise buttons
    set_cruiseEnable(false);
    set_cruiseSet(false);

    // Set velocityObserved
    set_velocityObserved(velocityObserved);
}

void goToReverseDrive(float velocityObserved){
    // Set reverse drive state & gear
    set_state(REVERSE_DRIVE);
    set_gear(REVERSE_GEAR);

    // Set cruise buttons
    set_cruiseEnable(false);
    set_cruiseSet(false);

    // Set velocityObserved
    set_velocityObserved(velocityObserved);
}

void goToPoweredCruise(float velocityObserved, float cruiseVelSetpoint){
    // Set powered cruise state & gear
    set_state(POWERED_CRUISE);
    set_gear(FORWARD_GEAR);

    // Set cruise buttons
    set_cruiseEnable(true);
    set_cruiseSet(false);

    // Set relevant state properties
    set_velocityObserved(velocityObserved);
    set_cruiseVelSetpoint(cruiseVelSetpoint); 
}

void goToCoastingCruise(float velocityObserved, float cruiseVelSetpoint){
    // Set power cruise state & gear
    set_state(COASTING_CRUISE);
    set_gear(FORWARD_GEAR);

    // Set cruise buttons
    set_cruiseEnable(true);
    set_cruiseSet(false);

    // Set relevant state properties
    set_velocityObserved(velocityObserved);
    set_cruiseVelSetpoint(cruiseVelSetpoint); 
}

void goToAccelerateCruise(float velocityObserved, float cruiseVelSetpoint){
    // Set power cruise state & gear
    set_state(ACCELERATE_CRUISE);
    set_gear(FORWARD_GEAR);

    // Set cruise buttons
    set_cruiseEnable(true);
    set_cruiseSet(false);

    // Set relevant state properties
    set_velocityObserved(velocityObserved);
    set_cruiseVelSetpoint(cruiseVelSetpoint);
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
    // set_regenEnable(ON);

    // OSTaskCreate(
    //     (OS_TCB*)&SendTritium_TCB,
    //     (CPU_CHAR*)"SendTritium",
    //     (OS_TASK_PTR)Task_SendTritium,
    //     (void*) NULL,
    //     (OS_PRIO)TASK_SEND_TRITIUM_PRIO,
    //     (CPU_STK*)SendTritium_Stk,
    //     (CPU_STK_SIZE)WATERMARK_STACK_LIMIT/10,
    //     (CPU_STK_SIZE)TASK_SEND_TRITIUM_STACK_SIZE,
    //     (OS_MSG_QTY) 0,
    //     (OS_TICK)NULL,
    //     (void*)NULL,
    //     (OS_OPT)(OS_OPT_TASK_STK_CLR),
    //     (OS_ERR*)&err
    // );
    //assertOSError(err);

    /**
     * ======= Forward Drive ==========
     * State Transitions: 
     * Powered Cruise, Park, Reverse Drive
    */
    printf("\n\r============ Testing Forward Drive State ============\n\r");

    // Forward Drive to Powered Cruise
    printf("\n\rTesting: Forward Drive -> Powered Cruise\n\r");
    // Ensure no switch when velocity < cruise threshold
    goToForwardDrive(VEL_BELOW_CRUISE_THRESH); 
    stateHandler();
    set_accelPedalPercent(ACCEL_PRESSED);
    set_brakePedalPercent(BRAKE_UNPRESSED);
    set_cruiseEnable(true);
    set_cruiseSet(true);
    stateDecider();
    while(get_state().name != FORWARD_DRIVE){}
    // Ensure no switch when brake is pressed
    stateHandler();
    set_velocityObserved(VEL_EXCEED_CRUISE_THRESH);
    set_brakePedalPercent(BRAKE_PRESSED);
    stateDecider();
    while(get_state().name != FORWARD_DRIVE){}
    // Ensure switch when all conditions are met
    stateHandler();
    set_brakePedalPercent(BRAKE_UNPRESSED);
    stateDecider();
    while(get_state().name != POWERED_CRUISE){}

    // Forward Drive to Park
    printf("\n\rTesting: Forward Drive -> Park\n\r");
    // Ensure change when park gear set
    goToForwardDrive(VEL_EXCEED_CRUISE_THRESH);
    stateHandler();
    set_gear(PARK_GEAR);
    stateDecider();
    while(get_state().name != PARK_STATE){}

    // Forward Drive to Reverse Drive
    printf("\n\rTesting: Forward Drive -> Reverse Drive\n\r");
    // Ensure change when reverse gear set
    goToForwardDrive(VEL_EXCEED_CRUISE_THRESH);
    stateHandler();
    set_gear(REVERSE_GEAR);
    stateDecider();
    stateHandler();
    stateDecider();
    while(get_state().name != REVERSE_DRIVE){}

    /**
     * ======= Park ==========
     * State Transitions: 
     * Forward Drive, Reverse Drive
    */

    printf("\n\r============ Testing Park State ============\n\r");

    // Park to Forward Drive
    printf("\n\rTesting: Park -> Forward Drive\n\r");
    // Ensure change when forward gear set
    goToParkState(VEL_BELOW_CRUISE_THRESH);
    stateHandler();
    set_gear(FORWARD_GEAR);
    stateDecider();
    while(get_state().name != FORWARD_DRIVE){}

    // Park to Reverse Drive
    printf("\n\rTesting: Park -> Reverse Drive\n\r");
    // Ensure change when reverse gear set
    goToParkState(VEL_BELOW_CRUISE_THRESH);
    stateHandler(); 
    set_gear(REVERSE_GEAR);
    stateDecider();
    while(get_state().name != REVERSE_DRIVE){}

    /**
     * ======= Reverse Drive ==========
     * State Transitions: 
     * Park, Forward Drive
    */

    printf("\n\r============ Testing Reverse Drive State ============\n\r");
    
    // Reverse Drive to Park
    printf("\n\rTesting: Reverse Drive -> Park\n\r");
    // Ensure change when park gear is set
    goToReverseDrive(VEL_BELOW_CRUISE_THRESH);
    stateHandler();
    set_gear(PARK_GEAR);
    stateDecider();
    while(get_state().name != PARK_STATE){}

    // Reverse Drive to Forward Drive
    printf("\n\rTesting: Reverse Drive -> Forward Drive\n\r");
    // Ensure change when forward gear is set
    goToReverseDrive(VEL_BELOW_CRUISE_THRESH);
    stateHandler();
    set_gear(FORWARD_GEAR);
    stateDecider();
    stateHandler();
    stateDecider();
    while(get_state().name != FORWARD_DRIVE){}

    /**
     * ======= Powered Cruise ==========
     * State Transitions: 
     * Forward Drive, Park, Reverse Drive, Accelerate Cruise, Coasting Cruise
    */
    printf("\n\r============ Testing Powered Cruise State ============\n\r");

    // Powered Cruise to Forward Drive
    printf("\n\rTesting: Powered Cruise -> Forward Drive\n\r");
    // Ensure change when cruiseEnable is unset
    goToPoweredCruise(VEL_EXCEED_CRUISE_THRESH - 1.0f, VEL_EXCEED_CRUISE_THRESH);
    stateHandler();
    set_brakePedalPercent(BRAKE_UNPRESSED);
    set_accelPedalPercent(ACCEL_UNPRESSED);
    set_cruiseEnable(false);
    stateDecider();
    while(get_state().name != FORWARD_DRIVE){}
    // Ensure change when brake is pressed
    goToPoweredCruise(VEL_EXCEED_CRUISE_THRESH - 1.0f, VEL_EXCEED_CRUISE_THRESH);
    stateHandler();
    set_brakePedalPercent(BRAKE_PRESSED);
    set_accelPedalPercent(ACCEL_UNPRESSED);
    stateDecider();
    while(get_state().name != FORWARD_DRIVE){}


    // Powered Cruise to Park State
    printf("\n\rTesting: Powered Cruise -> Park\n\r");
    // Ensure change when park gear is set
    goToPoweredCruise(VEL_EXCEED_CRUISE_THRESH - 1.0f, VEL_EXCEED_CRUISE_THRESH);
    stateHandler();
    set_brakePedalPercent(BRAKE_UNPRESSED);
    set_accelPedalPercent(ACCEL_UNPRESSED);
    set_gear(PARK_GEAR);
    stateDecider();
    while(get_state().name != PARK_STATE){}
    // Ensure change when reverse gear is set
    goToPoweredCruise(VEL_EXCEED_CRUISE_THRESH - 1.0f, VEL_EXCEED_CRUISE_THRESH);
    stateHandler();
    set_brakePedalPercent(BRAKE_UNPRESSED);
    set_accelPedalPercent(ACCEL_UNPRESSED);
    set_gear(REVERSE_GEAR);
    stateDecider();
    while(get_state().name != PARK_STATE){}   

    // Powered Cruise to Reverse Drive
    printf("\n\rTesting: Powered Cruise -> Reverse Drive\n\r");
    // Ensure change when reverse gear is set
    goToPoweredCruise(VEL_EXCEED_CRUISE_THRESH - 1.0f, VEL_EXCEED_CRUISE_THRESH);
    stateHandler();
    set_brakePedalPercent(BRAKE_UNPRESSED);
    set_accelPedalPercent(ACCEL_UNPRESSED);
    set_gear(REVERSE_GEAR);
    stateDecider();
    while(get_state().name != PARK_STATE){}   
    stateHandler();
    stateDecider();
    while(get_state().name != REVERSE_DRIVE){}

    // Powered Cruise to Accelerate Cruise
    printf("\n\rTesting: Powered Cruise -> Accelerate Cruise\n\r");
    // Velocity-controlled transition
    goToPoweredCruise(VEL_EXCEED_CRUISE_THRESH  - 1.0f, VEL_EXCEED_CRUISE_THRESH);
    stateHandler();
    set_brakePedalPercent(BRAKE_UNPRESSED);
    set_accelPedalPercent(ACCEL_PRESSED);  
    stateDecider();
    while(get_state().name != ACCELERATE_CRUISE){}

    // Powered Cruise to Coasting Cruise
    printf("\n\rTesting: Powered Cruise -> Coasting Cruise\n\r");
    // Velocity-controlled transition
    goToPoweredCruise(VEL_EXCEED_CRUISE_THRESH, VEL_EXCEED_CRUISE_THRESH  - 1.0f);
    stateHandler();
    set_brakePedalPercent(BRAKE_UNPRESSED);
    set_accelPedalPercent(ACCEL_UNPRESSED);  
    stateDecider();
    while(get_state().name != COASTING_CRUISE){}

    // Powered Cruise modifying cruiseVelSetpoint
    printf("\n\rTesting: Powered Cruise - Setting new cruise velocity\n\r");
    cruiseVelTestVar = VEL_EXCEED_CRUISE_THRESH; 
    goToPoweredCruise(cruiseVelTestVar, VEL_EXCEED_CRUISE_THRESH + 1.0f);
    set_cruiseSet(true);
    stateHandler();
    while(get_cruiseVelSetpoint() != cruiseVelTestVar) {};


    /**
     * ======= Coasting Cruise ==========
     * State Transitions: 
     * Forward Drive, Park, Reverse Drive, Accelerate Cruise, Powered Cruise
    */
    printf("\n\r============ Testing Coasting Cruise State ============\n\r");

    // Coasting Cruise to Forward Drive
    printf("\n\rTesting: Coasting Cruise -> Forward Drive\n\r");
    // Testing CRUZ_EN button transition
    goToCoastingCruise(VEL_EXCEED_CRUISE_THRESH, VEL_EXCEED_CRUISE_THRESH - 1.0f);
    stateHandler();
    set_brakePedalPercent(BRAKE_UNPRESSED);
    set_accelPedalPercent(ACCEL_UNPRESSED); 
    set_cruiseEnable(false);
    stateDecider();
    while(get_state().name != FORWARD_DRIVE){}
    // Testing brake pedal transition
    goToCoastingCruise(VEL_EXCEED_CRUISE_THRESH, VEL_EXCEED_CRUISE_THRESH - 1.0f); 
    stateHandler();
    set_brakePedalPercent(BRAKE_PRESSED);
    set_accelPedalPercent(ACCEL_UNPRESSED); 
    stateDecider();
    while(get_state().name != FORWARD_DRIVE){}

    // Coasting Cruise to Park
    printf("\n\rTesting: Coasting Cruise -> Park\n\r");
    // Ensure change when park gear is set
    goToCoastingCruise(VEL_EXCEED_CRUISE_THRESH, VEL_EXCEED_CRUISE_THRESH - 1.0f); 
    stateHandler();
    set_brakePedalPercent(BRAKE_UNPRESSED);
    set_accelPedalPercent(ACCEL_UNPRESSED); 
    set_gear(PARK_GEAR);
    stateDecider();
    while(get_state().name != PARK_STATE){}
    // Ensure change when reverse gear is set
    goToPoweredCruise(VEL_EXCEED_CRUISE_THRESH, VEL_EXCEED_CRUISE_THRESH - 1.0f);
    stateHandler();
    set_brakePedalPercent(BRAKE_UNPRESSED);
    set_accelPedalPercent(ACCEL_UNPRESSED);
    set_gear(REVERSE_GEAR);
    stateDecider();
    while(get_state().name != PARK_STATE){}   

    // Coasting Cruise to Reverse Drive
    printf("\n\rTesting: Coasting Cruise -> Reverse Drive\n\r");
    // Ensure change when reverse gear is set
    goToPoweredCruise(VEL_EXCEED_CRUISE_THRESH, VEL_EXCEED_CRUISE_THRESH - 1.0f);
    stateHandler();
    set_brakePedalPercent(BRAKE_UNPRESSED);
    set_accelPedalPercent(ACCEL_UNPRESSED);
    set_gear(REVERSE_GEAR);
    stateDecider();
    while(get_state().name != PARK_STATE){}   
    stateHandler();
    stateDecider();
    while(get_state().name != REVERSE_DRIVE){}

    // Coasting Cruise to Accelerate Cruise
    printf("\n\rTesting: Powered Cruise -> Accelerate Cruise\n\r");
    // Ensure change when accel pedal is pressed
    goToCoastingCruise(VEL_EXCEED_CRUISE_THRESH, VEL_EXCEED_CRUISE_THRESH - 1.0f);
    stateHandler();
    set_brakePedalPercent(BRAKE_UNPRESSED);
    set_accelPedalPercent(ACCEL_PRESSED);
    stateDecider();
    while(get_state().name != ACCELERATE_CRUISE){}

    // Coasting Cruise to Powered Cruise
    printf("\n\rTesting: Coasting Cruise -> Powered Cruise\n\r");
    goToCoastingCruise(VEL_EXCEED_CRUISE_THRESH - 1.0f, VEL_EXCEED_CRUISE_THRESH);
    stateHandler();
    set_brakePedalPercent(BRAKE_UNPRESSED);
    set_accelPedalPercent(ACCEL_UNPRESSED);
    stateDecider();
    while(get_state().name != POWERED_CRUISE){}

    // Coasting Cruise modifying cruiseVelSetpoint
    printf("\n\rTesting: Coasting Cruise - Setting new cruise velocity\n\r");
    cruiseVelTestVar = VEL_EXCEED_CRUISE_THRESH + 1.0f; 
    goToPoweredCruise(cruiseVelTestVar, VEL_EXCEED_CRUISE_THRESH);
    set_cruiseSet(true);
    stateHandler();
    while(get_cruiseVelSetpoint() != cruiseVelTestVar) {};

    /**
     * ======= Accelerate Cruise State ==========
     * State Transitions: Forward Drive, Park, Coasting Cruise
    */
    printf("\n\r============ Testing Accelerate Cruise State ============\n\r");

    // Accelerate Cruise to Forward Drive
    printf("\n\rTesting: Accelerate Cruise -> Forward Drive\n\r");
    // Ensure switch when CRUZ_EN is toggled off
    goToAccelerateCruise(VEL_EXCEED_CRUISE_THRESH + 1.0f, VEL_EXCEED_CRUISE_THRESH);
    stateHandler();
    set_brakePedalPercent(BRAKE_UNPRESSED);
    set_accelPedalPercent(ACCEL_UNPRESSED);
    set_cruiseEnable(false);
    stateDecider();
    while(get_state().name != FORWARD_DRIVE){}
    // Testing brake pedal transition
    goToAccelerateCruise(VEL_EXCEED_CRUISE_THRESH + 1.0f, VEL_EXCEED_CRUISE_THRESH); 
    stateHandler();
    set_brakePedalPercent(BRAKE_PRESSED);
    set_accelPedalPercent(ACCEL_UNPRESSED); 
    stateDecider();
    while(get_state().name != FORWARD_DRIVE){}

    // Accelerate Cruise to Park
    printf("\n\rTesting: Accelerate Cruise -> Park\n\r");
    // Ensure change when park gear is set
    goToAccelerateCruise(VEL_EXCEED_CRUISE_THRESH + 1.0f, VEL_EXCEED_CRUISE_THRESH);
    stateHandler();
    set_brakePedalPercent(BRAKE_UNPRESSED);
    set_accelPedalPercent(ACCEL_UNPRESSED);
    set_gear(PARK_GEAR);
    stateDecider();
    while(get_state().name != PARK_STATE){}
    // Ensure change when reverse gear is set
    goToAccelerateCruise(VEL_EXCEED_CRUISE_THRESH + 1.0f, VEL_EXCEED_CRUISE_THRESH);
    stateHandler();
    set_brakePedalPercent(BRAKE_UNPRESSED);
    set_accelPedalPercent(ACCEL_UNPRESSED);
    set_gear(REVERSE_GEAR);
    stateDecider();
    while(get_state().name != PARK_STATE){} 


    // Accelerate Cruise to Reverse Drive
    printf("\n\rTesting: Accelerate Cruise -> Reverse Drive\n\r");
    // Ensure change when reverse gear is set
    goToAccelerateCruise(VEL_EXCEED_CRUISE_THRESH + 1.0f, VEL_EXCEED_CRUISE_THRESH);
    stateHandler();
    set_brakePedalPercent(BRAKE_UNPRESSED);
    set_accelPedalPercent(ACCEL_UNPRESSED);
    set_gear(REVERSE_GEAR);
    stateDecider();
    while(get_state().name != PARK_STATE){}   
    stateHandler();
    stateDecider();
    while(get_state().name != REVERSE_DRIVE){}

    // Accelerate Cruise to Coasting Cruise
    printf("\n\rTesting: Accelerate Cruise -> Coasting Cruise\n\r");
    // Ensure transition when accel pedal is unpressed
    goToPoweredCruise(VEL_EXCEED_CRUISE_THRESH + 1.0f, VEL_EXCEED_CRUISE_THRESH);
    stateHandler();
    set_brakePedalPercent(BRAKE_UNPRESSED);
    set_accelPedalPercent(ACCEL_PRESSED);
    stateDecider();
    while(get_state().name != ACCELERATE_CRUISE){}
    stateHandler();
    set_accelPedalPercent(ACCEL_UNPRESSED);
    stateDecider();
    while(get_state().name != COASTING_CRUISE){}

    // Accelerate Cruise modifying cruiseVelSetpoint
    printf("\n\rTesting: Accelerate Cruise - Setting new cruise velocity\n\r");
    cruiseVelTestVar = VEL_EXCEED_CRUISE_THRESH + 1.0f; 
    goToAccelerateCruise(cruiseVelTestVar, VEL_EXCEED_CRUISE_THRESH);
    set_cruiseSet(true);
    stateHandler();
    while(get_cruiseVelSetpoint() != cruiseVelTestVar) {};


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