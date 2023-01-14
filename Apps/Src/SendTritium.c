/**
 * @copyright Copyright (c) 2022 UT Longhorn Racing Solar
 * 
 * @file SendTritium.c
 * @brief Function implementations for the SendTritium application.
 * 
 * This contains functions relevant to updating the velocity and current setpoitns
 * of the Tritium motor controller. The implementation includes a normal current
 * controlled mode, a one-pedal driving mode (with regenerative braking), and cruise control.
 * The logic is determined through a finite state machine implementation.
 * 
 * If the macro __TEST_SENDTRITIUM is defined prior to including SendTritium.h, relevant
 * variables will be exposed as externs for unit testing.
 * 
 * @author Nathaniel Delgado (NathanielDelgado)
 * @author Diya Rajon (diyarajon)
 * @author Ishan Deshpande (IshDeshpa, ishdeshpa@utexas.edu)
 */

#include "Pedals.h"
#include "ReadCarCAN.h"
#include "Minions.h"
#include "SendTritium.h"
#include "ReadTritium.h"
#include "CANbus.h"
#include "UpdateDisplay.h"

// Macros
#define MAX_VELOCITY 20000.0f // rpm (unobtainable value)
#define MIN_CRUISE_VELOCITY 20.0f // m/s
#define MAX_GEARSWITCH_VELOCITY 8.0f // m/s

#define BRAKE_PEDAL_THRESHOLD 5  // percent
#define ACCEL_PEDAL_THRESHOLD 10 // percent

#define ONEPEDAL_BRAKE_THRESHOLD 25 // percent
#define ONEPEDAL_NEUTRAL_THRESHOLD 35 // percent

#define GEAR_FAULT_THRESHOLD 3 // number of times gear fault can occur before it is considered a fault

#ifdef __TEST_SENDTRITIUM
#define STATIC(def) static def
#else
#define STATIC(def) def
#endif

// Inputs
STATIC(bool) cruiseEnable = false;
STATIC(bool) cruiseSet = false;
STATIC(bool) onePedalEnable = false;
STATIC(bool) regenEnable = false;

STATIC(uint8_t) brakePedalPercent = 0;
STATIC(uint8_t) accelPedalPercent = 0;

STATIC(Gear_t) gear = NEUTRAL_GEAR;

// Outputs
float currentSetpoint = 0;
float velocitySetpoint = 0;
float cruiseVelSetpoint = 0;

// Current observed velocity
STATIC(float) velocityObserved = 0;

// Counter for sending setpoints to motor
static uint8_t motorMsgCounter = 0;

#ifndef __TEST_SENDTRITIUM
// Debouncing counters
static uint8_t onePedalCounter = 0;
static uint8_t cruiseEnableCounter = 0;
static uint8_t cruiseSetCounter = 0;

// Button states
static bool onePedalButton = false;
static bool onePedalPrevious = false;

static bool cruiseEnableButton = false;
static bool cruiseEnablePrevious = false;

#endif

// Handler & Decider Declarations
static void ForwardDriveHandler(void);
static void ForwardDriveDecider(void);
static void NeutralDriveHandler(void);
static void NeutralDriveDecider(void);
static void ReverseDriveHandler(void);
static void ReverseDriveDecider(void);
static void RecordVelocityHandler(void);
static void RecordVelocityDecider(void);
static void PoweredCruiseHandler(void);
static void PoweredCruiseDecider(void);
static void CoastingCruiseHandler(void);
static void CoastingCruiseDecider(void);
static void BrakeHandler(void);
static void BrakeDecider(void);
static void OnePedalDriveHandler(void);
static void OnePedalDriveDecider(void);
static void AccelerateCruiseHandler(void);
static void AccelerateCruiseDecider(void);

// State Names
typedef enum{
    FORWARD_DRIVE,
    NEUTRAL_DRIVE,
    REVERSE_DRIVE,
    RECORD_VELOCITY,
    POWERED_CRUISE,
    COASTING_CRUISE,
    BRAKE_STATE,
    ONEPEDAL,
    ACCELERATE_CRUISE
} TritiumStateName_t;

// State Struct for FSM
typedef struct TritiumState{
    TritiumStateName_t name;
    void (*stateHandler)(void);
    void (*stateDecider)(void);
} TritiumState_t;

// FSM
static const TritiumState_t FSM[9] = {
    {FORWARD_DRIVE, &ForwardDriveHandler, &ForwardDriveDecider},
    {NEUTRAL_DRIVE, &NeutralDriveHandler, &NeutralDriveDecider},
    {REVERSE_DRIVE, &ReverseDriveHandler, &ReverseDriveDecider},
    {RECORD_VELOCITY, &RecordVelocityHandler, &RecordVelocityDecider},
    {POWERED_CRUISE, &PoweredCruiseHandler, &PoweredCruiseDecider},
    {COASTING_CRUISE, &CoastingCruiseHandler, &CoastingCruiseDecider},
    {BRAKE_STATE, &BrakeHandler, &BrakeDecider},
    {ONEPEDAL, &OnePedalDriveHandler, &OnePedalDriveDecider},
    {ACCELERATE_CRUISE, &AccelerateCruiseHandler, &AccelerateCruiseDecider}
};

static TritiumState_t prevState; // Previous state
static TritiumState_t state; // Current state

/**
 * @brief Dumps info to UART during testing
*/
#ifdef __TEST_SENDTRITIUM
static void dumpInfo(){
    printf("-------------------\n\r");
    printf("State: %d\n\r", state);
    printf("cruiseEnable: %d\n\r", cruiseEnable);
    printf("cruiseSet: %d\n\r", cruiseSet);
    printf("onePedalEnable: %d\n\r", onePedalEnable);
    printf("brakePedalPercent: %d\n\r", brakePedalPercent);
    printf("accelPedalPercent: %d\n\r", accelPedalPercent);
    printf("reverseSwitch: %d\n\r", reverseSwitch);
    printf("forwardSwitch: %d\n\r", forwardSwitch);
    printf("currentSetpoint: %f\n\r", currentSetpoint);
    printf("velocitySetpoint: %f\n\r", velocitySetpoint);
    printf("velocityObserved: %f\n\r", velocityObserved);
    printf("-------------------\n\r");
}
#endif

#ifndef __TEST_SENDTRITIUM
/**
 * @brief Reads inputs from the system
*/
static void readInputs(){
    Minion_Error_t err;

    // Update pedals
    brakePedalPercent = Pedals_Read(BRAKE);
    accelPedalPercent = Pedals_Read(ACCELERATOR);
    
    // Update regen enable
    regenEnable = ChargeEnable_Get();

    // Update buttons
    if(Minion_Read_Pin(REGEN_SW, &err) && onePedalCounter < DEBOUNCE_PERIOD){onePedalCounter++;}
    else if(onePedalCounter > 0){onePedalCounter--;}

    if(Minion_Read_Pin(CRUZ_EN, &err) && cruiseEnableCounter < DEBOUNCE_PERIOD){cruiseEnableCounter++;}
    else if(cruiseEnableCounter > 0){cruiseEnableCounter--;}

    if(Minion_Read_Pin(CRUZ_ST, &err) && cruiseSetCounter < DEBOUNCE_PERIOD){cruiseSetCounter++;}
    else if(cruiseSetCounter > 0){cruiseSetCounter--;}
    
    // Update gears
    bool forwardSwitch = Minion_Read_Pin(FOR_SW, &err);
    bool reverseSwitch = Minion_Read_Pin(REV_SW, &err);
    bool forwardGear = (forwardSwitch && !reverseSwitch);
    bool reverseGear = (!forwardSwitch && reverseSwitch);
    bool neutralGear = (!forwardSwitch && !reverseSwitch);

    uint8_t gearFault = (uint8_t) forwardGear + (uint8_t) reverseGear + (uint8_t) neutralGear;
    static uint8_t gearFaultCnt = 0;

    if(gearFault != 1){
        // Fault behavior
        if(gearFaultCnt > GEAR_FAULT_THRESHOLD) state = FSM[NEUTRAL_DRIVE];
        else gearFaultCnt++;
    }
    else{
        gearFaultCnt = 0;
    }

    if(neutralGear) gear = NEUTRAL_GEAR;
    else if(forwardGear) gear = FORWARD_GEAR;
    else if(reverseGear) gear = REVERSE_GEAR;
    else gear = NEUTRAL_GEAR;

    // Debouncing
    if(onePedalCounter == DEBOUNCE_PERIOD){onePedalButton = true;}
    else if(onePedalCounter == 0){onePedalButton = false;}

    if(cruiseEnableCounter == DEBOUNCE_PERIOD){cruiseEnableButton = true;}
    else if(cruiseEnableCounter == 0){cruiseEnableButton = false;}

    if(cruiseSetCounter == DEBOUNCE_PERIOD){cruiseSet = true;}
    else if(cruiseSetCounter == 0){cruiseSet = false;}

    // Toggle
    if(onePedalButton != onePedalPrevious && onePedalPrevious){onePedalEnable = !onePedalEnable;}
    if(!regenEnable) onePedalEnable = false;
    onePedalPrevious = onePedalButton;
    
    if(cruiseEnableButton != cruiseEnablePrevious && cruiseEnablePrevious){cruiseEnable = !cruiseEnable;}
    cruiseEnablePrevious = cruiseEnableButton;
    
    // Get observed velocity
    velocityObserved = Motor_Velocity_Get();
}
#endif

// Helper Functions

/**
 * @brief Converts integer percentage to float percentage
 * @param percent integer percentage from 0-100
 * @returns float percentage from 0.0-1.0
*/
extern const float pedalToPercent[];
static float percentToFloat(uint8_t percent){
    if(percent > 100){
        return 1.0f;
    }
    return pedalToPercent[percent];
}

/**
 * @brief Linearly map range of integers to another range of integers. 
 * in_min to in_max is mapped to out_min to out_max.
 * @param input input integer value
 * @param in_min minimum value of input range
 * @param in_max maximum value of input range
 * @param out_min minimum value of output range
 * @param out_max maximum value of output range 
 * @returns integer value from out_min to out_max
*/
static uint8_t map(uint8_t input, uint8_t in_min, uint8_t in_max, uint8_t out_min, uint8_t out_max){
    if(in_max >= in_min) in_max = in_min;
    if(input <= in_min) return out_min;
    else if(input >= in_max) return out_max;
    else return ((out_max - in_max) * (input - in_min))/(out_min - in_min) + out_min;
}


// State Handlers & Deciders

/**
 * @brief Forward Drive State Handler. Accelerator is mapped directly 
 * to current setpoint at positive velocity.
*/
static void ForwardDriveHandler(){
    if(prevState.name != state.name){
        UpdateDisplay_SetCruiseState(DISP_DISABLED);
        UpdateDisplay_SetRegenState(DISP_DISABLED);
        UpdateDisplay_SetGear(DISP_FORWARD);
    }
    velocitySetpoint = MAX_VELOCITY;
    currentSetpoint = percentToFloat(map(accelPedalPercent, ACCEL_PEDAL_THRESHOLD, 100, 0, 100));
}

/**
 * @brief Forward Drive State Decider. Determines transitions out of
 * forward drive state (brake, record velocity, one pedal, neutral drive).
*/
static void ForwardDriveDecider(){
    if(brakePedalPercent >= BRAKE_PEDAL_THRESHOLD){
        state = FSM[BRAKE];
    }else if(cruiseSet && cruiseEnable && velocityObserved >= MIN_CRUISE_VELOCITY){
        state = FSM[RECORD_VELOCITY];
    }else if(onePedalEnable){
        state = FSM[ONEPEDAL];
    }else if(gear == NEUTRAL_GEAR || gear == REVERSE_GEAR){
        state = FSM[NEUTRAL_DRIVE];
    }
}

/**
 * @brief Neutral Drive State Handler. No current is sent to the motor.
*/
static void NeutralDriveHandler(){
    if(prevState.name != state.name){
        UpdateDisplay_SetCruiseState(DISP_DISABLED);
        UpdateDisplay_SetRegenState(DISP_DISABLED);
        UpdateDisplay_SetGear(DISP_NEUTRAL);
    }
    velocitySetpoint = MAX_VELOCITY;
    currentSetpoint = 0.0f;

    cruiseEnable = false;
    onePedalEnable = false;
}

/**
 * @brief Neutral Drive State Decider. Determines transitions out of
 * neutral drive state (brake, forward drive, reverse drive).
*/
static void NeutralDriveDecider(){
    if(brakePedalPercent >= BRAKE_PEDAL_THRESHOLD){
        state = FSM[BRAKE];
    }else if(gear == FORWARD_GEAR && velocityObserved >= -MAX_GEARSWITCH_VELOCITY){
        state = FSM[FORWARD_DRIVE];
    }else if(gear == REVERSE_GEAR && velocityObserved <= MAX_GEARSWITCH_VELOCITY){
        state = FSM[REVERSE_DRIVE];
    }
}

/**
 * @brief Reverse Drive State Handler. Accelerator is mapped directly to 
 * current setpoint (at negative velocity).
*/
static void ReverseDriveHandler(){
    if(prevState.name != state.name){
        UpdateDisplay_SetCruiseState(DISP_DISABLED);
        UpdateDisplay_SetRegenState(DISP_DISABLED);
        UpdateDisplay_SetGear(DISP_REVERSE);
    }
    velocitySetpoint = -MAX_VELOCITY;
    currentSetpoint = percentToFloat(map(accelPedalPercent, ACCEL_PEDAL_THRESHOLD, 100, 0, 100));
    cruiseEnable = false;
    onePedalEnable = false;
}

/**
 * @brief Reverse Drive State Decider. Determines transitions out of
 * reverse drive state (brake, neutral drive).
*/
static void ReverseDriveDecider(){
    if(brakePedalPercent >= BRAKE_PEDAL_THRESHOLD){
        state = FSM[BRAKE];
    }
    else if(gear == NEUTRAL_GEAR || gear == FORWARD_GEAR){
        state = FSM[NEUTRAL_DRIVE];
    }
}

/**
 * @brief Record Velocity State. While pressing the cruise set button, 
 * the car will record the observed velocity into velocitySetpoint.
*/
static void RecordVelocityHandler(){
    if(prevState.name != state.name){
        UpdateDisplay_SetCruiseState(DISP_ACTIVE);
        UpdateDisplay_SetRegenState(DISP_DISABLED);
        UpdateDisplay_SetGear(DISP_FORWARD);
    }
    // put car in neutral while recording velocity (while button is held)
    velocitySetpoint = MAX_VELOCITY;
    currentSetpoint = 0;
    cruiseVelSetpoint = velocityObserved;
}

/**
 * @brief Record Velocity State Decider. Determines transitions out of record velocity 
 * state (brake, neutral drive, one pedal, forward drive, powered cruise).
*/
static void RecordVelocityDecider(){
    if(brakePedalPercent >= BRAKE_PEDAL_THRESHOLD){
        state = FSM[BRAKE_STATE];
    }else if(gear == NEUTRAL_GEAR || gear == REVERSE_GEAR){
        state = FSM[NEUTRAL_DRIVE];
    }else if(onePedalEnable){
        cruiseEnable = false;
        state = FSM[ONEPEDAL];
    }else if(!cruiseEnable){
        state = FSM[FORWARD_DRIVE];
    }else if(cruiseEnable && !cruiseSet){
        state = FSM[POWERED_CRUISE];
    }
}

/**
 * @brief Powered Cruise State. Continue to travel at the recorded velocity as long as
 * Observed Velocity <= Velocity Setpoint
*/
static void PoweredCruiseHandler(){
    velocitySetpoint = cruiseVelSetpoint;
    currentSetpoint = 1.0f;
}

/**
 * @brief Powered Cruise State Decider. Determines transitions out of powered 
 * cruise state (brake, neutral drive, one pedal, forward drive, record velocity, 
 * accelerate cruise, coasting cruise).
*/
static void PoweredCruiseDecider(){
    if(brakePedalPercent >= BRAKE_PEDAL_THRESHOLD){
        state = FSM[BRAKE_STATE];
    }else if(gear == NEUTRAL_GEAR || gear == REVERSE_GEAR){
        state = FSM[NEUTRAL_DRIVE];
    }else if(onePedalEnable){
        cruiseEnable = false;
        state = FSM[ONEPEDAL];
    }else if(!cruiseEnable){
        state = FSM[FORWARD_DRIVE];
    }else if(cruiseSet && velocityObserved >= MIN_CRUISE_VELOCITY){
        state = FSM[RECORD_VELOCITY];
    }else if(accelPedalPercent >= ACCEL_PEDAL_THRESHOLD){
        state = FSM[ACCELERATE_CRUISE];
    }else if(velocityObserved > cruiseVelSetpoint){
        state = FSM[COASTING_CRUISE];
    }
}

/**
 * @brief Coasting Cruise State. We do not want to utilize motor braking 
 * in cruise control mode due to safety issues. Coast the motor (go into neutral) 
 * if we want to slow down.
*/
static void CoastingCruiseHandler(){
    velocitySetpoint = cruiseVelSetpoint;
    currentSetpoint = 0;
}

/**
 * @brief Coasting Cruise State Decider. Determines transitions out of coasting 
 * cruise state (brake, neutral drive, one pedal, forward drive, record velocity, 
 * accelerate cruise, powered cruise).
*/
static void CoastingCruiseDecider(){
    if(brakePedalPercent >= BRAKE_PEDAL_THRESHOLD){
        state = FSM[BRAKE_STATE];
    }else if(gear == NEUTRAL_GEAR || gear == REVERSE_GEAR){
        state = FSM[NEUTRAL_DRIVE];
    }else if(onePedalEnable){
        cruiseEnable = false;
        state = FSM[ONEPEDAL];
    }else if(!cruiseEnable){
        state = FSM[FORWARD_DRIVE];
    }else if(cruiseSet && velocityObserved >= MIN_CRUISE_VELOCITY){
        state = FSM[RECORD_VELOCITY];
    }else if(accelPedalPercent >= ACCEL_PEDAL_THRESHOLD){
        state = FSM[ACCELERATE_CRUISE];
    }else if(velocityObserved <= cruiseVelSetpoint){
        state = FSM[POWERED_CRUISE];
    }
}

/**
 * @brief Accelerate Cruise State. In the event that the driver needs to accelerate in cruise
 * mode, we will accelerate to the pedal percentage. Upon release of the accelerator
 * pedal, we will return to cruise mode at the previously recorded velocity.
*/
static void AccelerateCruiseHandler(){
    velocitySetpoint = MAX_VELOCITY;
    currentSetpoint = percentToFloat(map(accelPedalPercent, ACCEL_PEDAL_THRESHOLD, 100, 0, 100));
}

/**
 * @brief Accelerate Cruise State Decider. Determines transitions out of accelerate 
 * cruise state (brake, neutral drive, one pedal, forward drive, record velocity, 
 * coasting cruise).
*/
static void AccelerateCruiseDecider(){
    if(brakePedalPercent >= BRAKE_PEDAL_THRESHOLD){
        state = FSM[BRAKE_STATE];
    }else if(gear == NEUTRAL_GEAR || gear == REVERSE_GEAR){
        state = FSM[NEUTRAL_DRIVE];
    }else if(onePedalEnable){
        cruiseEnable = false;
        state = FSM[ONEPEDAL];
    }else if(!cruiseEnable){
        state = FSM[FORWARD_DRIVE];
    }else if(cruiseSet && velocityObserved >= MIN_CRUISE_VELOCITY){
        state = FSM[RECORD_VELOCITY];
    }else if(accelPedalPercent < ACCEL_PEDAL_THRESHOLD){
        state = FSM[COASTING_CRUISE];
    }
}

/**
 * @brief One Pedal Drive State. When in one pedal drive, if the accelerator percentage is lower
 * than ONEPEDAL_BRAKE_THRESHOLD, the car will utilize motor braking to slow down. If accelerator
 * percentage is in the neutral zone, the car will coast. If accelerator percentage is above
 * the NEUTRAL_THRESHOLD, the car will accelerate as normal.
*/
static void OnePedalDriveHandler(){
    if(prevState.name != state.name){
        UpdateDisplay_SetCruiseState(DISP_DISABLED);
        UpdateDisplay_SetGear(DISP_FORWARD);
    }
    Minion_Error_t minion_err;
    if(accelPedalPercent <= ONEPEDAL_BRAKE_THRESHOLD){
        // Regen brake: Map 0 -> brake to 100 -> 0
        velocitySetpoint = 0;
        currentSetpoint = percentToFloat(map(accelPedalPercent, 0, ONEPEDAL_BRAKE_THRESHOLD, 100, 0));
        Minion_Write_Output(BRAKELIGHT, true, &minion_err);
        UpdateDisplay_SetRegenState(DISP_ACTIVE);
    }else if(ONEPEDAL_BRAKE_THRESHOLD < accelPedalPercent && accelPedalPercent <= ONEPEDAL_NEUTRAL_THRESHOLD){
        // Neutral: coast
        velocitySetpoint = MAX_VELOCITY;
        currentSetpoint = 0;
        Minion_Write_Output(BRAKELIGHT, false, &minion_err);
        UpdateDisplay_SetRegenState(DISP_ENABLED);
    }else if(ONEPEDAL_NEUTRAL_THRESHOLD < accelPedalPercent){
        // Accelerate: Map neutral -> 100 to 0 -> 100
        velocitySetpoint = MAX_VELOCITY;
        currentSetpoint = percentToFloat(map(accelPedalPercent, ONEPEDAL_NEUTRAL_THRESHOLD, 100, 0, 100));
        Minion_Write_Output(BRAKELIGHT, false, &minion_err);
        UpdateDisplay_SetRegenState(DISP_ENABLED);
    }
}

/**
 * @brief One Pedal Drive State Decider. Determines transitions out of one pedal 
 * drive state (brake, record velocity, neutral drive).
*/
static void OnePedalDriveDecider(){
    Minion_Error_t minion_err;
    if(brakePedalPercent >= BRAKE_PEDAL_THRESHOLD){
        state = FSM[BRAKE];
    }else if(cruiseSet && cruiseEnable && velocityObserved >= MIN_CRUISE_VELOCITY){
        state = FSM[RECORD_VELOCITY];
        Minion_Write_Output(BRAKELIGHT, false, &minion_err);
    }else if(gear == NEUTRAL_GEAR || gear == REVERSE_GEAR){
        state = FSM[NEUTRAL_DRIVE];
        Minion_Write_Output(BRAKELIGHT, false, &minion_err);
    }
}

/**
 * @brief Brake State. When brake pedal is pressed, physical brakes will be active.
 * Put motor in neutral to prevent motor braking while physical brakes are engaged.
 * Additionally, disable all cruise control and one pedal functionality.
*/
static void BrakeHandler(){
    if(prevState.name != state.name){
        UpdateDisplay_SetCruiseState(DISP_DISABLED);
        UpdateDisplay_SetRegenState(DISP_DISABLED);
        UpdateDisplay_SetGear(DISP_FORWARD);
    }
    Minion_Error_t minion_err;
    velocitySetpoint = MAX_VELOCITY;
    currentSetpoint = 0;
    cruiseEnable = false;
    onePedalEnable = false;
    Minion_Write_Output(BRAKELIGHT, true, &minion_err);
}

/**
 * @brief Brake State Decider. Determines transitions out of brake state (forward drive, 
 * neutral drive).
*/
static void BrakeDecider(){
    Minion_Error_t minion_err;
    if(brakePedalPercent < BRAKE_PEDAL_THRESHOLD){
        if(gear == FORWARD_GEAR) state = FSM[FORWARD_DRIVE];
        else if(gear == NEUTRAL_GEAR || gear == REVERSE_GEAR) state = FSM[NEUTRAL_DRIVE];
        Minion_Write_Output(BRAKELIGHT, false, &minion_err);
    }
}

// Task (main loop)

/**
 * @brief Follows the FSM to update the velocity of the car
*/
void Task_SendTritium(void *p_arg){
    OS_ERR err;

    // Initialize current state to FORWARD_DRIVE
    state = FSM[NEUTRAL_DRIVE];
    prevState = FSM[NEUTRAL_DRIVE];

    CANbus_Init(MOTORCAN);
    CANDATA_t driveCmd = {
        .ID=MOTOR_DRIVE, 
        .idx=0,
        .data={0.0f, 0.0f},
    };

    while(1){
        prevState = state;

        state.stateHandler();    // do what the current state does
        #ifndef __TEST_SENDTRITIUM
        readInputs();   // read inputs from the system
        UpdateDisplay_SetAccel(accelPedalPercent);
        #endif
        state.stateDecider();    // decide what the next state is

        // Drive
        #ifdef __TEST_SENDTRITIUM
        dumpInfo();
        #else
        if(MOTOR_MSG_COUNTER_THRESHOLD == motorMsgCounter){
            driveCmd.data[0] = currentSetpoint;
            driveCmd.data[1] = velocitySetpoint;
            CANbus_Send(driveCmd, CAN_NON_BLOCKING, MOTORCAN);
            motorMsgCounter = 0;
        }else{
            motorMsgCounter++;
        }
        #endif

        // Delay of MOTOR_MSG_PERIOD ms
        OSTimeDlyHMSM(0, 0, 0, MOTOR_MSG_PERIOD, OS_OPT_TIME_HMSM_STRICT, &err);
        if (err != OS_ERR_NONE){
            assertOSError(OS_UPDATE_VEL_LOC, err);
        }
    }
}
