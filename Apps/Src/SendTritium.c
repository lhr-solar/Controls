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
#include "MotorController.h"
#include "Minions.h"

// Macros
#define MAX_VELOCITY 50.0f  // m/s
#define BRAKE_THRESHOLD 20  // for one-pedal driving
#define NEUTRAL_THRESHOLD 25    // for one-pedal driving
#define BUTTON_BITMASK 0b111 // length of bitmap holding previous button values (in binary logic)

// Outputs
static float currentSetpoint = 0;
static float velocitySetpoint = 0;
static bool brakelightState = false;

// Inputs
static bool cruiseEnable = false;
static bool cruiseSet = false;
static bool regenToggle = false;
static uint8_t brakePedalPercent = 0;
static uint8_t accelPedalPercent = 0;
static bool reverseSwitch = false;
static bool forwardSwitch = false;

// Bitmap buffers
static uint8_t regenBitmap = 0;
static uint8_t cruiseEnableBitmap = 0;
static uint8_t cruiseSetBitmap = 0;


//button states
static bool regenButton = false;
static bool regenPrevious = false;

static bool cruiseEnableButton = false;
static bool cruiseEnablePrevious = false;


// State Names
typedef enum{
    NORMAL_DRIVE,
    RECORD_VELOCITY,
    POWERED_CRUISE,
    COASTING_CRUISE,
    BRAKE_STATE,
    ONEPEDAL_DRIVE,
    DISABLE_CRUISE,
    DISABLE_REGEN
} TritiumStateName_e;

// State Struct for FSM
typedef struct TritiumState{
    TritiumStateName_e name;
    void (*stateHandler)(void);
    void (*stateDecider)(void);
} TritiumState_t;

// Current state
static TritiumStateName_e state = NORMAL_DRIVE;

// Handler & Decider Declarations
void NormalDriveHandler(void);
void NormalDriveDecider(void);
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
void DisableCruiseHandler(void);
void DisableCruiseDecider(void);
void DisableRegenHandler(void);
void DisableRegenDecider(void);

// FSM
const TritiumState_t FSM[8] = {
    {NORMAL_DRIVE, &NormalDriveHandler, &NormalDriveDecider},
    {RECORD_VELOCITY, &RecordVelocityHandler, &RecordVelocityDecider},
    {POWERED_CRUISE, &PoweredCruiseHandler, &PoweredCruiseDecider},
    {COASTING_CRUISE, &CoastingCruiseHandler, &CoastingCruiseDecider},
    {BRAKE_STATE, &BrakeHandler, &BrakeDecider},
    {ONEPEDAL_DRIVE, &OnePedalDriveHandler, &OnePedalDriveDecider},
    {DISABLE_CRUISE, &DisableCruiseHandler, &DisableCruiseDecider},
    {DISABLE_REGEN, &DisableRegenHandler, &DisableRegenDecider}
};

/**
 * @brief   Reads inputs from the system
*/
void readInputs(){
    Minion_Error_t err;

    brakePedalPercent = Pedals_Read(BRAKE);
    accelPedalPercent = Pedals_Read(ACCELERATOR);
    
    // UPDATE BUTTONS
    if(Minion_Read_Input(REGEN_SW, &err)){regenBitmap |= 1;}
    if(Minion_Read_Input(CRUZ_EN, &err)){cruiseEnableBitmap |= 1;}
    if(Minion_Read_Input(CRUZ_ST, &err)){cruiseSetBitmap |= 1;}

    forwardSwitch = Minion_Read_Input(FOR_SW, &err);
    reverseSwitch = Minion_Read_Input(REV_SW, &err);

    if(forwardSwitch == ON){
        cruiseSet = Minion_Read_Input(CRUZ_ST, &err);
    }
    else{
        cruiseEnable = false;
        cruiseSet = false;
        regenToggle = false;
    }

    // DEBOUNCING
    regenBitmap &= BUTTON_BITMASK; //selecting the last n bits

    if(regenBitmap == BUTTON_BITMASK){
        regenButton = true;
    }else if(regenBitmap != BUTTON_BITMASK){
        regenButton = false;
    }

    cruiseEnableBitmap &= BUTTON_BITMASK; //selecting the last n bits

    if(cruiseEnableBitmap == BUTTON_BITMASK){
        cruiseEnableButton = true;
    }else if(cruiseEnableBitmap != BUTTON_BITMASK){
        cruiseEnableButton = false;
    }

    cruiseSetBitmap &= BUTTON_BITMASK; //selecting the last n bits

    if(cruiseSetBitmap == BUTTON_BITMASK){
        cruiseSet = true;
    }else if(cruiseEnableBitmap != BUTTON_BITMASK){
        cruiseSet = false;
    }

    // TOGGLE
    if(ChargeEnable && regenButton != regenPrevious && regenPrevious){regenToggle = !regenToggle;}
    regenPrevious = regenButton;
    
    if(cruiseEnableButton != cruiseEnablePrevious && cruiseEnablePrevious){cruiseEnable = !cruiseEnable;}
    cruiseEnablePrevious = cruiseEnableButton;

    // LEFT SHIFT
    cruiseEnableBitmap <<= 1; 
    regenBitmap <<= 1;
    cruiseSetBitmap <<= 1;
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
    
    while(1){
        FSM[state].stateHandler();    // do what the current state does
        readInputs();   // read inputs from the system
        FSM[state].stateDecider();    // decide what the next state is

        // Delay of few milliseconds (100)
        OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &err);
        if (err != OS_ERR_NONE){
            assertOSError(OS_UPDATE_VEL_LOC, err);
        }
        // Sets brakelight
        Minion_Write_Output(BRAKELIGHT, brakelightState, &minion_err);
    }
}

// State Handlers & Deciders

/**
 * Normal Drive State. Accelerator is mapped directly to current setpoint.
*/
void NormalDriveHandler(){
    if(reverseSwitch && !forwardSwitch){
        velocitySetpoint = -MAX_VELOCITY;
        currentSetpoint = percentToFloat(accelPedalPercent);
    }else if(!reverseSwitch && forwardSwitch){
        velocitySetpoint = MAX_VELOCITY;
        currentSetpoint = percentToFloat(accelPedalPercent);
    }
    else if(!reverseSwitch && !forwardSwitch){
        currentSetpoint = 0;
    }
    brakelightState = false;
}

void NormalDriveDecider(){
    if(cruiseSet && cruiseEnable && brakePedalPercent == 0){
        state = RECORD_VELOCITY;
    }else if(!cruiseEnable && regenToggle && ChargeEnable){
        state = ONEPEDAL_DRIVE;
    }else if(brakePedalPercent > 0){
        state = BRAKE_STATE;
    }
}

/**
 * Record Velocity State. While pressing the cruise set button, the car will record the observed velocity
 * into velocitySetpoint.
*/
void RecordVelocityHandler(){
    velocitySetpoint = MotorController_ReadVelocity();
    brakelightState = true;
}

void RecordVelocityDecider(){
    if(cruiseEnable && !cruiseSet){
        state = POWERED_CRUISE;
    }else if(!cruiseEnable){
        state = NORMAL_DRIVE;
    }else if(brakePedalPercent > 0){
        state = BRAKE_STATE;
    }
}

/**
 * Powered Cruise State. Continue to travel at the recorded velocity as long as
 * Observed Velocity <= Velocity Setpoint
*/
void PoweredCruiseHandler(){
    currentSetpoint = 1.0f;
    brakelightState = false;
}

void PoweredCruiseDecider(){
    if(MotorController_ReadVelocity() > velocitySetpoint){
        state = COASTING_CRUISE;
    }else if(!cruiseEnable){
        state = NORMAL_DRIVE;
    }else if(!cruiseEnable && !cruiseSet){
        state = RECORD_VELOCITY;
    }else if(brakePedalPercent > 0){
        state = BRAKE_STATE;
    }
}

/**
 * Coasting Cruise State. We do not want to utilize motor braking in cruise control mode.
 * Coast the motor (do not motor brake) if we want to slow down; instead, stop putting power
 * into the motor and let the car slow down naturally.
*/
void CoastingCruiseHandler(){
    currentSetpoint = 0;
    brakelightState = false;
}

void CoastingCruiseDecider(){
    if(MotorController_ReadVelocity() <= velocitySetpoint){
        state = POWERED_CRUISE;
    }else if(!cruiseEnable){
        state = NORMAL_DRIVE;
    }else if(brakePedalPercent > 0){
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
    if(brakePedalPercent == 0){
        state = ONEPEDAL_DRIVE;
    }
}

/**
 * One Pedal Drive State. When in one pedal drive, if the accelerator percentage is lower
 * than BRAKE_THRESHOLD, the car will utilize motor braking to slow down. If accelerator
 * percentage is in the neutral zone, the car will coast. If accelerator percentage is above
 * the NEUTRAL_THRESHOLD, the car will accelerate as normal.
*/
void OnePedalDriveHandler(){
    if(accelPedalPercent < BRAKE_THRESHOLD){
        // Motor brake
        currentSetpoint = percentToFloat(((BRAKE_THRESHOLD - accelPedalPercent)*100)/BRAKE_THRESHOLD);
        velocitySetpoint = 0;
        brakelightState = true;
    }
    else if(accelPedalPercent < NEUTRAL_THRESHOLD  && accelPedalPercent < BRAKE_THRESHOLD){
        // Coast
        currentSetpoint = 0;
    }else if(accelPedalPercent > NEUTRAL_THRESHOLD){
        // Accelerate
        currentSetpoint = percentToFloat(((accelPedalPercent - NEUTRAL_THRESHOLD)*100)/(100-NEUTRAL_THRESHOLD));
    }
}

void OnePedalDriveDecider(){
    if(!regenToggle || !ChargeEnable || cruiseEnable){
        state = DISABLE_REGEN;
    }else if(brakePedalPercent > 0){
        state = BRAKE_STATE;
    }
}

void DisableCruiseHandler(){
    cruiseEnable = 0;
}

void DisableCruiseDecider(){
    state = NORMAL_DRIVE;
}

void DisableRegenHandler(){
    regenToggle = 0;
}

void DisableRegenDecider(){
    state = NORMAL_DRIVE;
}