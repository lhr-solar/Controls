#include "Minions.h"
#include "Pedals.h"
#include "FaultState.h"
#include "CANbus.h"
#include "UpdateDisplay.h"
#include "ReadCarCAN.h"
#include "BSP_UART.h"
#include "Tasks.h"

#include "SendTritium_MVP_Cruise.h"

// Macros
#define VEL_EXCEED_GEARSWITCH_THRESH (MAX_GEARSWITCH_VELOCITY + 2.0f)
#define VEL_WITHIN_GEARSWITCH_THRESH 0.0f
#define VEL_EXCEED_NEG_GEARSWITCH_THRESH (-MAX_GEARSWITCH_VELOCITY - 2.0f)
#define VEL_EXCEED_MIN_CRUISE_THRESH (MIN_CRUISE_VELOCITY + 2.0f)
#define VEL_BELOW_MIN_CRUISE_THRESH 0.0f

#define ACCEL_UNPRESSED 0
#define ACCEL_PRESSED 100

#define BRAKE_UNPRESSED 0
#define BRAKE_PRESSED 100


static OS_TCB Task1TCB;
static CPU_STK Task1Stk[DEFAULT_STACK_SIZE];

void stateBuffer(){
    OS_ERR err;
    OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &err);
    assertOSError(err);
}

/**
 * ======= Testing Utilities ==========
 * Wrappers to set velocity, accel, & brake according to enum parameters &
 * to call state deciders/switch states
*/
void stateDecider() {
    TritiumStateName_t stateName = get_state();
    switch (stateName) {
        case FORWARD_DRIVE: callForwardDriveDecider();
        case PARK_STATE: callParkDecider();
        case REVERSE_DRIVE: callReverseDriveDecider();
        case RECORD_VELOCITY: callRecordVelocityDecider();
        case POWERED_CRUISE: callPoweredCruiseDecider();
        case COASTING_CRUISE: callCoastingCruiseDecider();
        case ACCELERATE_CRUISE: callAccelerateCruiseDecider();
    }    
}

/**
 * ======= State Setters ==========
 * States:
 * Forward Drive, Park State, Reverse Drive, RecordVelocity,
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

void goToRecordVelocity(float velocityObserved, uint8_t brakePedalPercent, uint8_t accelPedalPercent){
    // Set record velocity state & gear
    set_state(RECORD_VELOCITY);
    set_gear(FORWARD_GEAR);

    // Set cruise buttons
    set_cruiseEnable(true);
    set_cruiseSet(true);

    // Set relevant state properties
    set_velocityObserved(velocityObserved);
    set_cruiseVelSetpoint(velocityObserved);
    set_brakePedalPercent(brakePedalPercent);
    set_accelPedalPercent(accelPedalPercent);
}

void goToPoweredCruise(float velocityObserved, float cruiseVelSetpoint, uint8_t brakePedalPercent, uint8_t accelPedalPercent){
    // Set power cruise state & gear
    set_state(POWERED_CRUISE);
    set_gear(FORWARD_GEAR);

    // Set cruise buttons
    set_cruiseEnable(true);
    set_cruiseSet(false);

    // Set relevant state properties
    set_velocityObserved(velocityObserved);
    set_cruiseVelSetpoint(cruiseVelSetpoint); 
    set_brakePedalPercent(brakePedalPercent);
    set_accelPedalPercent(accelPedalPercent);
}

void goToCoastingCruise(float velocityObserved, float cruiseVelSetpoint, uint8_t brakePedalPercent, uint8_t accelPedalPercent){
    // Set power cruise state & gear
    set_state(COASTING_CRUISE);
    set_gear(FORWARD_GEAR);

    // Set cruise buttons
    set_cruiseEnable(true);
    set_cruiseSet(false);

    // Set relevant state properties
    set_velocityObserved(velocityObserved);
    set_cruiseVelSetpoint(cruiseVelSetpoint); 
    set_brakePedalPercent(brakePedalPercent);
    set_accelPedalPercent(accelPedalPercent);
}

void goToAccelerateCruise(float velocityObserved, float cruiseVelSetpoint, uint8_t brakePedalPercent, uint8_t accelPedalPercent){
    // Set power cruise state & gear
    set_state(ACCELERATE_CRUISE);
    set_gear(FORWARD_GEAR);

    // Set cruise buttons
    set_cruiseEnable(true);
    set_cruiseSet(false);

    // Set relevant state properties
    set_velocityObserved(velocityObserved);
    set_cruiseVelSetpoint(cruiseVelSetpoint);
    set_brakePedalPercent(brakePedalPercent);
    set_accelPedalPercent(accelPedalPercent);
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
     * Record Velocity, Park, Reverse Drive
    */
    printf("\n\r============ Testing Forward Drive State ============\n\r");

    // Forward Drive to Record Velocity
    printf("\n\rTesting: Forward Drive -> Record Velocity\n\r");
    goToForwardDrive();
    set_cruiseEnable(true);
    set_cruiseSet(true);
    set_velocityObserved(VEL_BELOW_MIN_CRUISE_THRESH);
    stateDecider();
    while(get_state().name != FORWARD_DRIVE){}
    set_velocityObserved(VEL_EXCEED_MIN_CRUISE_THRESH);
    stateDecider();
    while(get_state().name != RECORD_VELOCITY){}

    // Forward Drive to Park
    printf("\n\rTesting: Forward Drive -> Park\n\r");
    goToForwardDrive(VEL_EXCEED_GEARSWITCH_THRESH); // Velocity doesn't matter, gear-based transition
    set_gear(PARK_GEAR);
    stateDecider();
    while(get_state().name != PARK_STATE){}

    // Forward Drive to Reverse Drive
    printf("\n\rTesting: Forward Drive -> Reverse Drive\n\r");
    goToForwardDrive(VEL_WITHIN_GEARSWITCH_THRESH); // Velocity w/in Gearswitch Threshold
    set_gear(REVERSE_GEAR);
    stateDecider();
    stateDecider();
    while(get_state().name != REVERSE_DRIVE){}
    goToForwardDrive(VEL_EXCEED_GEARSWITCH_THRESH);  // Velocity > Gearswitch Threshold
    set_gear(REVERSE_GEAR);
    stateDecider();
    stateDecider();
    while(get_state().name != PARK_STATE){}

    /**
     * ======= Park ==========
     * State Transitions: 
     * Forward Drive, Reverse Drive
    */

    printf("\n\r============ Testing Park State ============\n\r");

    // Park to Forward Drive
    printf("\n\rTesting: Park -> Forward Drive\n\r");
    goToParkState(VEL_EXCEED_GEARSWITCH_THRESH);    // Velocity > Gearswitch Threshold  
    set_gear(FORWARD_GEAR);
    stateDecider();
    while(get_state().name != FORWARD_DRIVE){}
    goToParkState(VEL_WITHIN_GEARSWITCH_THRESH);    // Velocity w/in Gearswitch Threshold  
    set_gear(FORWARD_GEAR);
    stateDecider();
    while(get_state().name != FORWARD_DRIVE){}
    goToParkState(VEL_EXCEED_NEG_GEARSWITCH_THRESH); // Velocity < -Gearswitch Threshold
    set_gear(FORWARD_GEAR);
    stateDecider();
    while(get_state().name != PARK_STATE){}

    // Park to Reverse Drive
    printf("\n\rTesting: Park -> Reverse Drive\n\r");
    goToParkState(VEL_EXCEED_GEARSWITCH_THRESH);    // Velocity > Gearswitch Threshold  
    set_gear(REVERSE_GEAR);
    stateDecider();
    while(get_state().name != PARK_STATE){}
    goToParkState(VEL_WITHIN_GEARSWITCH_THRESH);    // Velocity w/in Gearswitch Threshold  
    set_gear(REVERSE_GEAR);
    stateDecider();
    while(get_state().name != REVERSE_DRIVE){}
    goToParkState(VEL_EXCEED_NEG_GEARSWITCH_THRESH); // Velocity < -Gearswitch Threshold
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
    goToReverseDrive(VEL_EXCEED_NEG_GEARSWITCH_THRESH); // Velocity doesn't matter, gear-based transition
    set_gear(PARK_GEAR);
    stateDecider();
    while(get_state().name != PARK_STATE){}

    // Reverse Drive to Forward Drive
    printf("\n\rTesting: Reverse Drive -> Forward Drive\n\r");
    goToReverseDrive(VEL_WITHIN_GEARSWITCH_THRESH);     // Velocity w/in Gearswitch Threshold
    set_gear(FORWARD_GEAR);
    stateDecider();
    while(get_state().name != FORWARD_DRIVE){}
    goToReverseDrive(VEL_EXCEED_NEG_GEARSWITCH_THRESH);  // Velocity < -Gearswitch Threshold  
    set_gear(FORWARD_GEAR);
    stateDecider();
    stateDecider();
    while(get_state().name != PARK_STATE){}

    /**
     * ======= Record Velocity ==========
     * State Transitions: 
     * Forward Drive, Park, Reverse Drive, Powered Cruise
    */
    printf("\n\r============ Testing Record Velocity State ============\n\r");

    // Record Velocity to Forward Normal Drive
    printf("\n\rTesting: Record Velocity -> Forward Drive\n\r");
    goToRecordVelocity(VEL_EXCEED_MIN_CRUISE_THRESH, BRAKE_UNPRESSED, ACCEL_UNPRESSED); // Velocity doesn't matter, gear-based transition
    set_cruiseEnable(false);
    stateDecider();
    while(get_state().name != FORWARD_DRIVE){}

    // Record Velocity to Park State
    printf("\n\rTesting: Record Velocity -> Park\n\r");
    goToRecordVelocity(VEL_EXCEED_MIN_CRUISE_THRESH, BRAKE_UNPRESSED, ACCEL_UNPRESSED); // Velocity doesn't matter, gear-based transition
    set_gear(PARK_GEAR);
    stateDecider();
    while(get_state().name != PARK_STATE){}
    goToRecordVelocity(VEL_EXCEED_MIN_CRUISE_THRESH, BRAKE_PRESSED, ACCEL_UNPRESSED); // Velocity doesn't matter, pedal-based transition
    stateDecider();
    while(get_state().name != PARK_STATE){}

    // Record Velocity to Reverse Drive
    printf("\n\rTesting: Record Velocity -> Reverse Drive\n\r");
    goToRecordVelocity(VEL_EXCEED_MIN_CRUISE_THRESH, BRAKE_UNPRESSED, ACCEL_UNPRESSED); // Min Cruise Vel > Max Gearswitch Vel
    set_gear(REVERSE_GEAR);
    stateDecider();
    stateDecider();
    while(get_state().name != PARK_STATE){}
    goToRecordVelocity(VEL_EXCEED_MIN_CRUISE_THRESH, BRAKE_UNPRESSED, ACCEL_UNPRESSED);
    set_gear(REVERSE_GEAR);
    stateDecider();
    set_velocityObserved(VEL_WITHIN_GEARSWITCH_THRESH);
    stateDecider();
    while(get_state().name != REVERSE_DRIVE){}
    
    // Record Velocity to Powered Cruise
    printf("\n\rTesting: Record Velocity -> Powered Cruise\n\r");
    goToRecordVelocity(VEL_EXCEED_MIN_CRUISE_THRESH, BRAKE_UNPRESSED, ACCEL_UNPRESSED);
    set_cruiseEnable(true);
    set_cruiseSet(false);
    stateDecider();
    while(get_state().name != POWERED_CRUISE){}

    /**
     * ======= Powered Cruise ==========
     * State Transitions: 
     * Forward Drive, Park, Reverse Drive, Record Velocity, Accelerate Cruise, Coasting Cruise
    */
    printf("\n\r============ Testing Powered Cruise State ============\n\r");

    // Powered Cruise to Forward Drive
    printf("\n\rTesting: Powered Cruise -> Forward Drive\n\r");
    goToPoweredCruise(VEL_EXCEED_GEARSWITCH_THRESH, VEL_EXCEED_GEARSWITCH_THRESH - 1.0f, BRAKE_UNPRESSED, ACCEL_UNPRESSED); // Velocity doesn't matter, button-controlled transition
    set_cruiseEnable(false);
    stateDecider();
    while(get_state().name != FORWARD_DRIVE){}

    // Powered Cruise to Park State
    printf("\n\rTesting: Powered Cruise -> Park\n\r");
    goToPoweredCruise(VEL_EXCEED_GEARSWITCH_THRESH, VEL_EXCEED_GEARSWITCH_THRESH - 1.0f, BRAKE_UNPRESSED, ACCEL_UNPRESSED); // Velocity doesn't matter, gear-based transition
    set_gear(PARK_GEAR);
    stateDecider();
    while(get_state().name != PARK_STATE){}
    goToPoweredCruise(VEL_EXCEED_GEARSWITCH_THRESH, VEL_EXCEED_GEARSWITCH_THRESH - 1.0f, BRAKE_PRESSED, ACCEL_UNPRESSED); // Velocity doesn't matter, pedal-based transition
    stateDecider();
    while(get_state().name != PARK_STATE){}   

    // Powered Cruise to Reverse Drive
    printf("\n\rTesting: Powered Cruise -> Reverse Drive\n\r");
    goToPoweredCruise(VEL_EXCEED_GEARSWITCH_THRESH, VEL_EXCEED_GEARSWITCH_THRESH - 1.0f, BRAKE_UNPRESSED, ACCEL_UNPRESSED); // Min Cruise Vel > Max Gearswitch Vel
    set_gear(REVERSE_GEAR);
    stateDecider();
    stateDecider();
    while(get_state().name != PARK_STATE){}
    goToPoweredCruise(VEL_EXCEED_GEARSWITCH_THRESH, VEL_EXCEED_GEARSWITCH_THRESH - 1.0f, BRAKE_UNPRESSED, ACCEL_UNPRESSED); // Min Cruise Vel > Max Gearswitch Vel
    set_gear(REVERSE_GEAR);
    stateDecider();
    set_velocityObserved(VEL_WITHIN_GEARSWITCH_THRESH);
    stateDecider();
    while(get_state().name != REVERSE_DRIVE){}

    // Powered Cruise to Record Velocity
    printf("\n\rTesting: Powered Cruise -> Record Velocity\n\r");
    goToPoweredCruise(VEL_EXCEED_GEARSWITCH_THRESH, VEL_EXCEED_GEARSWITCH_THRESH - 1.0f, BRAKE_UNPRESSED, ACCEL_UNPRESSED); // Velocity doesn't matter, button-controlled transition
    set_cruiseSet(true);
    stateDecider();
    while(get_state().name != RECORD_VELOCITY){}
    goToPoweredCruise(VEL_BELOW_MIN_CRUISE_THRESH, VEL_EXCEED_GEARSWITCH_THRESH, BRAKE_UNPRESSED, ACCEL_UNPRESSED); 
    set_cruiseSet(true);
    stateDecider();
    while(get_state().name != POWERED_CRUISE){}

    // Powered Cruise to Accelerate Cruise
    printf("\n\rTesting: Powered Cruise -> Accelerate Cruise\n\r");
    goToPoweredCruise(VEL_EXCEED_GEARSWITCH_THRESH, VEL_EXCEED_GEARSWITCH_THRESH - 1.0f, BRAKE_UNPRESSED, ACCEL_PRESSED); // Velocity doesn't matter, pedal-controlled transition
    stateDecider();
    while(get_state().name != ACCELERATE_CRUISE){}

    // Powered Cruise to Coasting Cruise
    printf("\n\rTesting: Powered Cruise -> Coasting Cruise\n\r");
    goToPoweredCruise(VEL_EXCEED_GEARSWITCH_THRESH, VEL_EXCEED_GEARSWITCH_THRESH - 1.0f, BRAKE_UNPRESSED, ACCEL_UNPRESSED);
    stateDecider();
    while(get_state().name != COASTING_CRUISE){}

    /**
     * ======= Coasting Cruise ==========
     * State Transitions: 
     * Forward Drive, Park, Reverse Drive, Record Velocity, Accelerate Cruise, Powered Cruise
    */
    printf("\n\r============ Testing Coasting Cruise State ============\n\r");

    // Coasting Cruise to Forward Drive
    printf("\n\rTesting: Coasting Cruise -> Forward Drive\n\r");
    goToCoastingCruise(VEL_EXCEED_GEARSWITCH_THRESH, VEL_EXCEED_GEARSWITCH_THRESH - 1.0f, BRAKE_UNPRESSED, ACCEL_UNPRESSED);
    set_cruiseEnable(false);
    stateDecider();
    while(get_state().name != FORWARD_DRIVE){}

    // Coasting Cruise to Park
    printf("\n\rTesting: Coasting Cruise -> Park\n\r");
    goToCoastingCruise(VEL_EXCEED_GEARSWITCH_THRESH, VEL_EXCEED_GEARSWITCH_THRESH - 1.0f, BRAKE_UNPRESSED, ACCEL_UNPRESSED); // Velocity doesn't matter, gear-based transition
    set_gear(PARK_GEAR);
    stateDecider();
    while(get_state().name != PARK_STATE){}
    goToCoastingCruise(VEL_EXCEED_GEARSWITCH_THRESH, VEL_EXCEED_GEARSWITCH_THRESH - 1.0f, BRAKE_PRESSED, ACCEL_UNPRESSED); // Velocity doesn't matter, pedal-based transition
    stateDecider();
    while(get_state().name != PARK_STATE){} 

    // Coasting Cruise to Reverse Drive
    printf("\n\rTesting: Coasting Cruise -> Reverse Drive\n\r");
    goToCoastingCruise(VEL_EXCEED_GEARSWITCH_THRESH, VEL_EXCEED_GEARSWITCH_THRESH - 1.0f, BRAKE_UNPRESSED, ACCEL_UNPRESSED); // Min Cruise Vel > Max Gearswitch Vel
    set_gear(REVERSE_GEAR);
    stateDecider();
    stateDecider();
    while(get_state().name != PARK_STATE){}
    goToCoastingCruise(VEL_EXCEED_GEARSWITCH_THRESH, VEL_EXCEED_GEARSWITCH_THRESH - 1.0f, BRAKE_UNPRESSED, ACCEL_UNPRESSED); // Min Cruise Vel > Max Gearswitch Vel
    set_gear(REVERSE_GEAR);
    stateDecider();
    set_velocityObserved(VEL_WITHIN_GEARSWITCH_THRESH);
    stateDecider();
    while(get_state().name != REVERSE_DRIVE){}

    // Coasting Cruise to Record Velocity
    printf("\n\rTesting: Coasting Cruise -> Record Velocity\n\r");
    goToCoastingCruise(VEL_EXCEED_GEARSWITCH_THRESH, VEL_EXCEED_GEARSWITCH_THRESH - 1.0f, BRAKE_UNPRESSED, ACCEL_UNPRESSED); // Velocity doesn't matter, button-controlled transition
    set_cruiseSet(true);
    stateDecider();
    while(get_state().name != RECORD_VELOCITY){}
    goToCoastingCruise(VEL_BELOW_MIN_CRUISE_THRESH, VEL_EXCEED_GEARSWITCH_THRESH, BRAKE_UNPRESSED, ACCEL_UNPRESSED); 
    set_cruiseSet(true);
    stateDecider();
    while(get_state().name != POWERED_CRUISE){}

    // Coasting Cruise to Accelerate Cruise
    printf("\n\rTesting: Powered Cruise -> Accelerate Cruise\n\r");
    goToCoastingCruise(VEL_EXCEED_GEARSWITCH_THRESH, VEL_EXCEED_GEARSWITCH_THRESH - 1.0f, BRAKE_UNPRESSED, ACCEL_PRESSED); // Velocity doesn't matter, pedal-controlled transition
    stateDecider();
    while(get_state().name != ACCELERATE_CRUISE){}

    // Coasting Cruise to Powered Cruise
    printf("\n\rTesting: Coasting Cruise -> Powered Cruise\n\r");
    goToCoastingCruise(VEL_EXCEED_GEARSWITCH_THRESH - 1.0f, VEL_EXCEED_GEARSWITCH_THRESH, BRAKE_UNPRESSED, ACCEL_UNPRESSED);
    stateDecider();
    while(get_state().name != POWERED_CRUISE){}

    /**
     * ======= Accelerate Cruise State ==========
     * State Transitions: Forward Drive, Park, Coasting Cruise, Record Velocity
    */
    printf("\n\r============ Testing Accelerate Cruise State ============\n\r");

    // Accelerate Cruise to Forward Drive
    printf("\n\rTesting: Accelerate Cruise -> Forward Drive\n\r");
    goToAccelerateCruise(VEL_EXCEED_GEARSWITCH_THRESH, VEL_EXCEED_GEARSWITCH_THRESH - 1.0f, BRAKE_UNPRESSED, ACCEL_UNPRESSED);
    set_cruiseEnable(false);
    stateDecider();
    while(get_state().name != FORWARD_DRIVE){}

    // Accelerate Cruise to Park
    printf("\n\rTesting: Accelerate Cruise -> Park\n\r");
    goToAccelerateCruise(VEL_EXCEED_GEARSWITCH_THRESH, VEL_EXCEED_GEARSWITCH_THRESH - 1.0f, BRAKE_UNPRESSED, ACCEL_UNPRESSED); // Velocity doesn't matter, gear-based transition
    set_gear(PARK_GEAR);
    stateDecider();
    while(get_state().name != PARK_STATE){}
    goToAccelerateCruise(VEL_EXCEED_GEARSWITCH_THRESH, VEL_EXCEED_GEARSWITCH_THRESH - 1.0f, BRAKE_PRESSED, ACCEL_UNPRESSED); // Velocity doesn't matter, pedal-based transition
    stateDecider();
    while(get_state().name != PARK_STATE){} 


    // Accelerate Cruise to Reverse Drive
    printf("\n\rTesting: Accelerate Cruise -> Reverse Drive\n\r");
    goToAccelerateCruise(VEL_EXCEED_GEARSWITCH_THRESH, VEL_EXCEED_GEARSWITCH_THRESH - 1.0f, BRAKE_UNPRESSED, ACCEL_UNPRESSED); // Min Cruise Vel > Max Gearswitch Vel
    set_gear(REVERSE_GEAR);
    stateDecider();
    stateDecider();
    while(get_state().name != PARK_STATE){}
    goToAccelerateCruise(VEL_EXCEED_GEARSWITCH_THRESH, VEL_EXCEED_GEARSWITCH_THRESH - 1.0f, BRAKE_UNPRESSED, ACCEL_UNPRESSED); // Min Cruise Vel > Max Gearswitch Vel
    set_gear(REVERSE_GEAR);
    stateDecider();
    set_velocityObserved(VEL_WITHIN_GEARSWITCH_THRESH);
    stateDecider();
    while(get_state().name != REVERSE_DRIVE){}

    // Accelerate Cruise to Record Velocity
    printf("\n\rTesting: Accelerate Cruise -> Record Velocity\n\r");
    goToAccelerateCruise(VEL_EXCEED_GEARSWITCH_THRESH, VEL_EXCEED_GEARSWITCH_THRESH - 1.0f, BRAKE_UNPRESSED, ACCEL_PRESSED); // Velocity doesn't matter, button-controlled transition
    set_cruiseSet(true);
    stateDecider();
    while(get_state().name != RECORD_VELOCITY){}
    goToAccelerateCruise(VEL_BELOW_MIN_CRUISE_THRESH, VEL_EXCEED_GEARSWITCH_THRESH, BRAKE_UNPRESSED, ACCEL_PRESSED); 
    set_cruiseSet(true);
    stateDecider();
    while(get_state().name != ACCELERATE_CRUISE){}

    // Accelerate Cruise to Coasting Cruise
    printf("\n\rTesting: Accelerate Cruise -> Coasting Cruise\n\r");
    goToPoweredCruise(VEL_EXCEED_GEARSWITCH_THRESH, VEL_EXCEED_GEARSWITCH_THRESH - 1.0f, BRAKE_UNPRESSED, ACCEL_UNPRESSED); // Velocity doesn't matter, pedal-based transition
    stateDecider();
    while(get_state().name != COASTING_CRUISE){}
    goToPoweredCruise(VEL_EXCEED_GEARSWITCH_THRESH - 1.0f, VEL_EXCEED_GEARSWITCH_THRESH, BRAKE_UNPRESSED, ACCEL_UNPRESSED); // Velocity doesn't matter, pedal-based transition
    stateDecider();
    while(get_state().name != COASTING_CRUISE){}

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