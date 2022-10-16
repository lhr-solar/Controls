/**
 * @copyright Copyright (c) 2022 UT Longhorn Racing Solar
 * 
 * @file UpdateVelocity.c
 * @brief Function implementations for the UpdateVelocity application.
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

// Setpoints
static float CurrentSetpoint = 0;
static float VelocitySetpoint = 0;

// Inputs
static bool CruiseEnable = false;
static bool CruiseSet = false;
static bool RegenButton = false;
static uint8_t BrakePedalPercent = 0;
static uint8_t AccelPedalPercent = 0;

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
} TritiumStateName_t;

// State Struct for FSM
typedef struct State{
    TritiumStateName_t name;
    void (*stateHandler)(void);
    void (*stateDecider)(void);
} TritiumState_t;

// Current state
static TritiumStateName_t state = NORMAL_DRIVE;

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
    // TODO: Change upon update to Minions driver
    BrakePedalPercent = Pedals_Read(BRAKE);
    AccelPedalPercent = Pedals_Read(ACCELERATOR);

    if(Switches_Read(FOR_SW) == ON){
        CruiseEnable = Switches_Read(CRUZ_EN)==ON?true:false;
        CruiseSet = Switches_Read(CRUZ_ST)==ON?true:false;
        RegenButton = Switches_Read(REGEN_SW)==ON?true:false;
    }
    else{
        CruiseEnable = false;
        CruiseSet = false;
        RegenButton = false;
    }
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
void Task_UpdateVelocity(void *p_arg){
    OS_ERR err;
    
    while(1){
        FSM[state].stateHandler();    // do what the current state does
        readInputs();   // read inputs from the system
        FSM[state].stateDecider();    // decide what the next state is

        // Delay of few milliseconds (100)
        OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &err);
        if (err != OS_ERR_NONE){
            assertOSError(OS_UPDATE_VEL_LOC, err);
        }
    }
}

// State Handlers & Deciders

/**
 * Normal Drive State. Accelerator is mapped directly to current setpoint.
*/
void NormalDriveHandler(){
    if(Switches_Read(REV_SW) == ON && Switches_Read(FOR_SW) == OFF){
        VelocitySetpoint = -MAX_VELOCITY;
        CurrentSetpoint = percentToFloat(AccelPedalPercent);
    }else if(Switches_Read(REV_SW) == OFF && Switches_Read(FOR_SW) == ON){
        VelocitySetpoint = MAX_VELOCITY;
        CurrentSetpoint = percentToFloat(AccelPedalPercent);
    }
    else if(Switches_Read(REV_SW) == OFF && Switches_Read(FOR_SW) == OFF){
        CurrentSetpoint = 0;
    }
    Lights_Set(BrakeLight, OFF);
}

void NormalDriveDecider(){
    if(CruiseSet && CruiseEnable && BrakePedalPercent == 0){
        state = RECORD_VELOCITY;
    }else if(!CruiseEnable && RegenButton && ChargeEnable){
        state = ONEPEDAL_DRIVE;
    }else if(BrakePedalPercent > 0){
        state = BRAKE_STATE;
    }
}

/**
 * Record Velocity State. While pressing the cruise set button, the car will record the observed velocity
 * into VelocitySetpoint.
*/
void RecordVelocityHandler(){
    VelocitySetpoint = MotorController_ReadVelocity();
    Lights_Set(BrakeLight, OFF);
}

void RecordVelocityDecider(){
    if(CruiseEnable && !CruiseSet){
        state = POWERED_CRUISE;
    }else if(!CruiseEnable){
        state = NORMAL_DRIVE;
    }else if(BrakePedalPercent > 0){
        state = BRAKE_STATE;
    }
}

/**
 * Powered Cruise State. Continue to travel at the recorded velocity as long as
 * Observed Velocity <= Velocity Setpoint
*/
void PoweredCruiseHandler(){
    CurrentSetpoint = 1.0f;
    Lights_Set(BrakeLight, OFF);
}

void PoweredCruiseDecider(){
    if(MotorController_ReadVelocity() > VelocitySetpoint){
        state = COASTING_CRUISE;
    }else if(!CruiseEnable){
        state = NORMAL_DRIVE;
    }else if(!CruiseEnable && !CruiseSet){
        state = RECORD_VELOCITY;
    }else if(BrakePedalPercent > 0){
        state = BRAKE_STATE;
    }
}

/**
 * Coasting Cruise State. We do not want to utilize motor braking in cruise control mode.
 * Coast the motor (do not motor brake) if we want to slow down; instead, stop putting power
 * into the motor and let the car slow down naturally.
*/
void CoastingCruiseHandler(){
    CurrentSetpoint = 0;
    Lights_Set(BrakeLight, OFF);
}

void CoastingCruiseDecider(){
    if(MotorController_ReadVelocity() <= VelocitySetpoint){
        state = POWERED_CRUISE;
    }else if(!CruiseEnable){
        state = NORMAL_DRIVE;
    }else if(BrakePedalPercent > 0){
        state = BRAKE_STATE;
    }
}

/**
 * Brake State. Brake the car if the brake pedal is pressed.
*/
void BrakeHandler(){
    CurrentSetpoint = 0;
    VelocitySetpoint = 0;
    CruiseEnable = 0;
    CruiseSet = 0;
    Lights_Set(BrakeLight, ON);
}

void BrakeDecider(){
    if(BrakePedalPercent == 0){
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
    if(AccelPedalPercent < BRAKE_THRESHOLD){
        // Motor brake
        CurrentSetpoint = percentToFloat(((BRAKE_THRESHOLD - AccelPedalPercent)*100)/BRAKE_THRESHOLD);
        VelocitySetpoint = 0;
        Lights_Set(BrakeLight, ON);
    }
    else if(AccelPedalPercent < NEUTRAL_THRESHOLD  && AccelPedalPercent < BRAKE_THRESHOLD){
        // Coast
        CurrentSetpoint = 0;
    }else if(AccelPedalPercent > NEUTRAL_THRESHOLD){
        // Accelerate
        CurrentSetpoint = percentToFloat(((AccelPedalPercent - NEUTRAL_THRESHOLD)*100)/(100-NEUTRAL_THRESHOLD));
    }
}

void OnePedalDriveDecider(){
    if(!RegenButton || !ChargeEnable || CruiseEnable){
        state = DISABLE_REGEN;
    }else if(BrakePedalPercent > 0){
        state = BRAKE_STATE;
    }
}

void DisableCruiseHandler(){
    CruiseEnable = 0;
}

void DisableCruiseDecider(){
    state = NORMAL_DRIVE;
}

void DisableRegenHandler(){
    RegenButton = 0;
}

void DisableRegenDecider(){
    state = NORMAL_DRIVE;
}