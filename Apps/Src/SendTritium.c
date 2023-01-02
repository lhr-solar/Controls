/**
 * @copyright Copyright (c) 2022 UT Longhorn Racing Solar
 * 
 * @file SendTritium.c
 * @brief Function implementations for the SendTritium application.
 * 
 * This contains functions relevant to updating the velocity and current setpoitns
 * of the Tritium motor controller. The implementation includes a normal current
 * controlled mode, a one-pedal driving mode, cruise control, and regenerative braking.
 * The logic is determined through a finite state machine implementation.
 * 
 * @author Nathaniel Delgado (NathanielDelgado)
 * @author Diya Rajon (diyarajon)
 * @author Ishan Deshpande (IshDeshpa)
 */

#include "Pedals.h"
#include "ReadCarCAN.h"
#include "Minions.h"
#include "SendTritium.h"
#include "ReadTritium.h"
#include "CANbus.h"
#include "UpdateDisplay.h"

// Macros
#define MAX_VELOCITY 50.0f // m/s    if rmp: 20000.0f // rpm
#define MIN_CRUISE_VELOCITY 20.0f // m/s
#define MAX_GEARSWITCH_VELOCITY 5.0f // m/s

#define BRAKE_PEDAL_THRESHOLD 5  // percent
#define ACCEL_CRUISE_THRESHOLD 10 // percent

#define CRUISE_CURR_LIMIT 1.0f // A

#define MOTOR_MSG_PERIOD 100
#define FSM_PERIOD 20
#define DEBOUNCE_PERIOD 2 // in units of FSM_PERIOD
#define MOTOR_MSG_COUNTER_THRESHOLD (MOTOR_MSG_PERIOD)/(FSM_PERIOD)

// Inputs
static bool cruiseEnable = false;
static bool cruiseSet = false;
static bool onePedalEnable = false;
static bool chargeEnable = false;

static uint8_t brakePedalPercent = 0;
static uint8_t accelPedalPercent = 0;

static bool forwardGear = false;
static bool neutralGear = false;
static bool reverseGear = false;

// Outputs
static float currentSetpoint = 0;
static float velocitySetpoint = 0;
static float cruiseVelSetpoint = 0;
static bool brakelightState = false;

// Current observed velocity
static float velocityObserved = 0;

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
void ForwardDriveHandler(void);
void ForwardDriveDecider(void);
void NeutralDriveHandler(void);
void NeutralDriveDecider(void);
void ReverseDriveHandler(void);
void ReverseDriveDecider(void);
void RecordVelocityHandler(void);
void RecordVelocityDecider(void);
void PoweredCruiseHandler(void);
void PoweredCruiseDecider(void);
void CoastingCruiseHandler(void);
void CoastingCruiseDecider(void);
void BrakeHandler(void);
void BrakeDecider(void);
void OnePedalDriveHandler(void);
void OnePedalDriveDecider(void);
void AccelerateCruiseHandler(void);
void AccelerateCruiseDecider(void);

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
    void (*stateHandler)(void);
    void (*stateDecider)(void);
} TritiumState_t;

// FSM
static const TritiumState_t FSM[9] = {
    {&ForwardDriveHandler, &ForwardDriveDecider},
    {&NeutralDriveHandler, &NeutralDriveDecider},
    {&ReverseDriveHandler, &ReverseDriveDecider},
    {&RecordVelocityHandler, &RecordVelocityDecider},
    {&PoweredCruiseHandler, &PoweredCruiseDecider},
    {&CoastingCruiseHandler, &CoastingCruiseDecider},
    {&BrakeHandler, &BrakeDecider},
    {&OnePedalDriveHandler, &OnePedalDriveDecider},
    {&AccelerateCruiseHandler, &AccelerateCruiseDecider}
};

// Current state
static TritiumState_t state;

/**
 * @brief Dumps info to UART during testing
*/
#ifdef __TEST_SENDTRITIUM
void dumpInfo(){
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

/**
 * @brief Reads inputs from the system
*/
void readInputs(){
    #ifndef __TEST_SENDTRITIUM
    Minion_Error_t err;

    // Update pedals
    brakePedalPercent = Pedals_Read(BRAKE);
    accelPedalPercent = Pedals_Read(ACCELERATOR);
    
    // Update charge enable
    chargeEnable = RegenEnable_Get();

    // Update buttons
    if(Minion_Read_Input(REGEN_SW, &err) && onePedalCounter < DEBOUNCE_PERIOD){onePedalCounter++;}
    else if(onePedalCounter > 0){onePedalCounter--;}

    if(Minion_Read_Input(CRUZ_EN, &err) && cruiseEnableCounter < DEBOUNCE_PERIOD){cruiseEnableCounter++;}
    else if(cruiseEnableCounter > 0){cruiseEnableCounter--;}

    if(Minion_Read_Input(CRUZ_ST, &err) && cruiseSetCounter < DEBOUNCE_PERIOD){cruiseSetCounter++;}
    else if(cruiseSetCounter > 0){cruiseSetCounter--;}
    
    // Update gears
    bool forwardSwitch = Minion_Read_Input(FOR_SW, &err);
    bool reverseSwitch = Minion_Read_Input(REV_SW, &err);
    forwardGear = (forwardSwitch && !reverseSwitch);
    reverseGear = (!forwardSwitch && reverseSwitch);
    neutralGear = (!forwardSwitch && !reverseSwitch);

    uint8_t gearFault = (uint8_t) forwardGear + (uint8_t) reverseGear + (uint8_t) neutralGear;
    
    if(gearFault != 1){
        // TODO: Determine fault behavior
    }

    // Debouncing
    if(onePedalCounter == DEBOUNCE_PERIOD){onePedalButton = true;}
    else if(onePedalCounter == 0){onePedalButton = false;}

    if(cruiseEnableCounter == DEBOUNCE_PERIOD){cruiseEnableButton = true;}
    else if(cruiseEnableCounter == 0){cruiseEnableButton = false;}

    if(cruiseSetCounter == DEBOUNCE_PERIOD){cruiseSet = true;}
    else if(cruiseSetCounter == 0){cruiseSet = false;}

    // Toggle
    if(chargeEnable && onePedalButton != onePedalPrevious && onePedalPrevious){onePedalEnable = !onePedalEnable;}
    onePedalPrevious = onePedalButton;
    
    if(cruiseEnableButton != cruiseEnablePrevious && cruiseEnablePrevious){cruiseEnable = !cruiseEnable;}
    cruiseEnablePrevious = cruiseEnableButton;
    
    // Get observed velocity
    velocityObserved = MotorVelocity_Get();

    #endif
}

/**
 * @brief Converts integer percentage to float percentage
 * @param percent integer percentage from 0-100
*/
extern const float pedalToPercent[];
static float percentToFloat(uint8_t percent){
    if(percent > 100){
        return 1.0f;
    }
    return pedalToPercent[percent];
}

/**
 * @brief Follows the FSM to update the velocity of the car
*/
void Task_SendTritium(void *p_arg){
    OS_ERR err;
    Minion_Error_t minion_err;

    // Initialize current state to NORMAL_DRIVE
    state = FSM[FORWARD_DRIVE];

    CANbus_Init(MOTORCAN);
    CANDATA_t driveCmd = {
        .ID=MOTOR_VELOCITY, 
        .idx=0,
        .data=0,
    };

    // #ifndef __TEST_SENDTRITIUM
    // CANPayload_t velocityPayload;
    // velocityPayload.bytes = 32;
    // velocityPayload.data.d = velocitySetpoint;

    // CANPayload_t currentPayload;
    // currentPayload.bytes = 32;
    // currentPayload.data.d = currentSetpoint;
    // #endif

    while(1){
        state.stateHandler();    // do what the current state does
        readInputs();   // read inputs from the system
        state.stateDecider();    // decide what the next state is

        // Sets brakelight
        Minion_Write_Output(BRAKELIGHT, brakelightState, &minion_err);

        // Drive
        #ifndef __TEST_SENDTRITIUM
        if(MOTOR_MSG_COUNTER_THRESHOLD == motorMsgCounter){
            CANbus_Send(&driveCmd, CAN_NON_BLOCKING, MOTORCAN);
            motorMsgCounter = 0;
        }else{
            motorMsgCounter++;
        }
        #endif

        // Dump Info for Testing
        #ifdef __TEST_SENDTRITIUM
        dumpInfo();
        #endif

        // Delay of few milliseconds (100)
        OSTimeDlyHMSM(0, 0, 0, MOTOR_MSG_PERIOD, OS_OPT_TIME_HMSM_STRICT, &err);
        if (err != OS_ERR_NONE){
            assertOSError(OS_UPDATE_VEL_LOC, err);
        }
    }
}

// State Handlers & Deciders

/**
 * Forward Drive State. Accelerator is mapped directly to current setpoint.
*/
void ForwardDriveHandler(){
    velocitySetpoint = MAX_VELOCITY;
    currentSetpoint = percentToFloat(accelPedalPercent);
}

void ForwardNormalDriveDecider(){
    if(brakePedalPercent >= BRAKE_PEDAL_THRESHOLD){
        state = FSM[BRAKE];
    }else if(cruiseSet && cruiseEnable && velocityObserved >= MIN_CRUISE_VELOCITY){
        state = FSM[RECORD_VELOCITY];
    }else if(onePedalEnable && chargeEnable){
        state = FSM[ONEPEDAL];
    }else if(neutralGear || reverseGear){
        state = FSM[NEUTRAL_DRIVE];
    }
}

/**
 * Neutral Drive State. No current is sent to the motor.
 * 
*/
void NeutralDriveHandler(){
    velocitySetpoint = MAX_VELOCITY;
    cruiseEnable = false;
    onePedalEnable = false;
    currentSetpoint = 0.0f;
}

void NeutralDriveDecider(){
    if(brakePedalPercent >= BRAKE_PEDAL_THRESHOLD){
        state = FSM[BRAKE];
    }else if(forwardGear && velocityObserved < MAX_GEARSWITCH_VELOCITY){
        state = FSM[FORWARD_DRIVE];
    }else if(reverseGear && velocityObserved < MAX_GEARSWITCH_VELOCITY){
        state = FSM[REVERSE_DRIVE];
    }
}

/**
 * Reverse Drive State. Accelerator is mapped directly to current setpoint (at negative velocity).
*/
void ReverseDriveHandler(){
    velocitySetpoint = -MAX_VELOCITY;
    currentSetpoint = percentToFloat(accelPedalPercent);
}

void ReverseDriveDecider(){
    if(brakePedalPercent >= BRAKE_PEDAL_THRESHOLD){
        state = FSM[BRAKE];
    }
    else if(neutralGear || forwardGear){
        state = FSM[NEUTRAL_DRIVE];
    }
}

/**
 * Record Velocity State. While pressing the cruise set button, the car will record the observed velocity
 * into velocitySetpoint.
*/
void RecordVelocityHandler(){
    cruiseVelSetpoint = velocityObserved;
}

void RecordVelocityDecider(){
    if(brakePedalPercent >= BRAKE_PEDAL_THRESHOLD){
        state = FSM[BRAKE_STATE];
    }else if(neutralGear){
        state = FSM[NEUTRAL_DRIVE];
    }else if(reverseGear){
        state = FSM[REVERSE_DRIVE];
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
 * Powered Cruise State. Continue to travel at the recorded velocity as long as
 * Observed Velocity <= Velocity Setpoint
*/
void PoweredCruiseHandler(){
    velocitySetpoint = cruiseVelSetpoint;
    currentSetpoint = CRUISE_CURR_LIMIT;
}

void PoweredCruiseDecider(){
    if(brakePedalPercent >= BRAKE_PEDAL_THRESHOLD){
        state = FSM[BRAKE_STATE];
    }else if(neutralGear){
        state = FSM[NEUTRAL_DRIVE];
    }else if(reverseGear){
        state = FSM[REVERSE_DRIVE];
    }else if(onePedalEnable){
        cruiseEnable = false;
        state = FSM[ONEPEDAL];
    }else if(!cruiseEnable){
        state = FSM[FORWARD_DRIVE];
    }else if(cruiseSet && velocityObserved >= MIN_CRUISE_VELOCITY){
        state = FSM[RECORD_VELOCITY];
    }else if(accelPedalPercent >= ACCEL_CRUISE_THRESHOLD){
        state = FSM[ACCELERATE_CRUISE];
    }else if(velocityObserved > cruiseVelSetpoint){
        state = FSM[COASTING_CRUISE];
    }
}

/**
 * Coasting Cruise State. We do not want to utilize motor braking in cruise control mode.
 * Coast the motor (do not motor brake) if we want to slow down; instead, stop putting current
 * into the motor and let the car slow down naturally.
*/
void CoastingCruiseHandler(){
    velocitySetpoint = cruiseVelSetpoint;
    currentSetpoint = 0;
}

void CoastingCruiseDecider(){
    if(brakePedalPercent >= BRAKE_PEDAL_THRESHOLD){
        state = FSM[BRAKE_STATE];
    }else if(neutralGear){
        state = FSM[NEUTRAL_DRIVE];
    }else if(reverseGear){
        state = FSM[REVERSE_DRIVE];
    }else if(onePedalEnable){
        cruiseEnable = false;
        state = FSM[ONEPEDAL];
    }else if(!cruiseEnable){
        state = FSM[FORWARD_DRIVE];
    }else if(cruiseSet && velocityObserved >= MIN_CRUISE_VELOCITY){
        state = FSM[RECORD_VELOCITY];
    }else if(accelPedalPercent >= ACCEL_CRUISE_THRESHOLD){
        state = FSM[ACCELERATE_CRUISE];
    }else if(velocityObserved <= cruiseVelSetpoint){
        state = FSM[POWERED_CRUISE];
    }
}

/**
 * One Pedal Drive State. When in one pedal drive, if the accelerator percentage is lower
 * than ONEPEDAL_BRAKE_THRESHOLD, the car will utilize motor braking to slow down. If accelerator
 * percentage is in the neutral zone, the car will coast. If accelerator percentage is above
 * the NEUTRAL_THRESHOLD, the car will accelerate as normal.
*/
void OnePedalDriveHandler(){
    if(accelPedalPercent < ONEPEDAL_BRAKE_THRESHOLD){
        // Motor brake
        currentSetpoint = percentToFloat(((ONEPEDAL_BRAKE_THRESHOLD - accelPedalPercent)*100)/ONEPEDAL_BRAKE_THRESHOLD);
        velocitySetpoint = 0;
        brakelightState = true;
    }else if(accelPedalPercent < NEUTRAL_THRESHOLD  && accelPedalPercent >= ONEPEDAL_BRAKE_THRESHOLD){
        // Coast
        currentSetpoint = 0;
        brakelightState = false;
    }else if(accelPedalPercent >= NEUTRAL_THRESHOLD){
        // Accelerate
        currentSetpoint = percentToFloat(((accelPedalPercent - NEUTRAL_THRESHOLD)*100)/(100-NEUTRAL_THRESHOLD));
        velocitySetpoint = MAX_VELOCITY
        brakelightState = false;
    }
}

void OnePedalDriveDecider(){
    if(!onePedalEnable || !chargeEnable || cruiseEnable){
        onePedalEnable = 0;
        state = NORMAL_DRIVE;
    }else if(brakePedalPercent >= NORMAL_BRAKE_THRESHOLD){
        state = BRAKE_STATE;
    }
}


void AccelerateCruiseHandler(){
    if(reverseSwitch && !forwardSwitch){
        velocitySetpoint = -MAX_VELOCITY;
        currentSetpoint = percentToFloat(accelPedalPercent);
    }else if(!reverseSwitch && forwardSwitch){
        velocitySetpoint = MAX_VELOCITY;
        currentSetpoint = percentToFloat(accelPedalPercent);
    }else if(!reverseSwitch && !forwardSwitch){
        currentSetpoint = 0;
    }
}

void AccelerateCruiseDecider(){
    if(accelPedalPercent == 0){
        state = COASTING_CRUISE;
    }else if(brakePedalPercent >= NORMAL_BRAKE_THRESHOLD){
        state = BRAKE_STATE;
    }
}

/**
 * Brake State. Brake the car if the brake pedal is pressed.
*/
void BrakeHandler(){
    currentSetpoint = 0;
    velocitySetpoint = 0;
    cruiseEnable = 0;
    cruiseSet = 0;
    brakelightState = true;
}

void BrakeDecider(){
    if(brakePedalPercent < NORMAL_BRAKE_THRESHOLD){
        state = NORMAL_DRIVE;
        brakelightState = false;
    }
}