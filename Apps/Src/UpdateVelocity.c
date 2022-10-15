/**
 * @file UpdateVelocity(1).c
 * @author Nathaniel Delgado (nathaniel.delgado@utexas.edu)
 * @brief 
 * @version 0.1
 * @date 2022-09-17
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "Pedals.h"
#include "UpdateVelocity.h"
#include "ReadCarCAN.h"
#include "MotorController.h"
#include "Minions.h"

// MAX_VELOCITY IN M/S
#define MAX_VELOCITY 50.0f
#define BRAKE_THRESHOLD 20
#define NEUTRAL_THRESHOLD 25


static uint8_t CurrentSetpoint = 0;
static uint32_t VelocitySetpoint = 0;
static bool CruiseEnable = false;
static bool CruiseSet = false;
static bool RegenButton = false;
static uint8_t BrakePedalPercent = 0;
static uint8_t AccelPedalPercent = 0;


extern const float pedalToPercent[];
static float convertPedaltoMotorPercent(uint8_t pedalPercentage){
    if(pedalPercentage > 100){
        return 1.0f;
    }
    return pedalToPercent[pedalPercentage];
}

// Convert a float velocity in meters per second to a desired RPM
static float velocity_to_rpm(float velocity) {
    float velocity_mpm = velocity * 60.0f; // velocity in meters per minute
    const float circumference = WHEEL_DIAMETER * M_PI;
    float wheel_rpm = velocity_mpm / circumference;
    return wheel_rpm;
}

typedef enum{
    NORMAL_DRIVE,
    RECORD_VELOCITY,
    POWERED_CRUISE,
    COASTING_CRUISE,
    BRAKE,
    REGEN_DRIVE,
    DISABLE_REGEN, // redirects to cruise
    DISABLE_CRUISE
} Tritium_state_name_t;

Tritium_state_name_t state = NORMAL_DRIVE;

// Struct to encapsulate each state. Keeps track of what the state does and how to decide the next state
typedef struct State{
    Tritium_state_name_t name;
    void (*stateHandler)(void);
    void (*stateDecider)(void);
} Tritium_state_t;

void NormalDriveHandler(){
    if(Switches_Read(REV_SW) == ON && Switches_Read(FOR_SW) == OFF){
        VelocitySetpoint = -MAX_VELOCITY;
        CurrentSetpoint = convertPedaltoMotorPercent(AccelPedalPercent);
    }else if(Switches_Read(REV_SW) == OFF && Switches_Read(FOR_SW) == ON){
        VelocitySetpoint = MAX_VELOCITY;
        CurrentSetpoint = convertPedaltoMotorPercent(AccelPedalPercent);
    }
}

void NormalDriveDecider(){
    if(CruiseSet && CruiseEnable && BrakePedalPercent == 0){
        state = RECORD_VELOCITY;
    }else if(!CruiseEnable && RegenButton && ChargeEnable){
        state = REGEN_DRIVE;
    }else if(BrakePedalPercent > 0){
        state = BRAKE;
    }
}

void RecordVelocityHandler(){
    VelocitySetpoint = MotorController_ReadVelocity();
}

void RecordVelocityDecider(){
    if(CruiseEnable && !CruiseSet){
        state = POWERED_CRUISE;
    }else if(!CruiseEnable){
        state = NORMAL_DRIVE;
    }else if(BrakePedalPercent > 0){
        state = BRAKE;
    }
}

void PoweredCruiseHandler(){
    CurrentSetpoint = 1.0f;
}

void PoweredCruiseDecider(){
    if(MotorController_ReadVelocity() > VelocitySetpoint){
        state = COASTING_CRUISE;
    }else if(!CruiseEnable){
        state = NORMAL_DRIVE;
    }else if(!CruiseEnable && !CruiseSet){
        state = RECORD_VELOCITY;
    }else if(BrakePedalPercent > 0){
        state = BRAKE;
    }
}

void CoastingCruiseHandler(){
    CurrentSetpoint = 0.0f;
}

void CoastingCruiseDecider(){
    if(MotorController_ReadVelocity() <= VelocitySetpoint){
        state = POWERED_CRUISE;
    }else if(!CruiseEnable){
        state = NORMAL_DRIVE;
    }else if(BrakePedalPercent > 0){
        state = BRAKE;
    }
}

void BrakeHandler(){
    CurrentSetpoint = 0;
    VelocitySetpoint = 0;
    CruiseEnable = 0;
    CruiseSet = 0;
}

void BrakeDecider(){
    if(BrakePedalPercent == 0){
        state = REGEN_DRIVE;
    }
}

void RegenDriveHandler(){
    if(AccelPedalPercent > NEUTRAL_THRESHOLD){
        CurrentSetpoint = (AccelPedalPercent - NEUTRAL_THRESHOLD)*(100/(100-NEUTRAL_THRESHOLD));
    }else if(AccelPedalPercent < NEUTRAL_THRESHOLD  && AccelPedalPercent < BRAKE_THRESHOLD){
        CurrentSetpoint = 0;
    }else if(AccelPedalPercent < BRAKE_THRESHOLD){
        CurrentSetpoint = (BRAKE_THRESHOLD - AccelPedalPercent)*(100/BRAKE_THRESHOLD);
        VelocitySetpoint = 0    
    }
}

void RegenDriveDecider(){
    if(CruiseEnable){
        state = DISABLE_CRUISE;
    }else if(!RegenButton && !ChargeEnable){
        state = DISABLE_CRUISE;
    }else if(BrakePedalPercent > 0){
        state = BRAKE;
    }
}

void DisableCruiseHandler(){
    CruiseEnable = 0;
}
void DisableCruiseDecider(){
    state = REGEN_DRIVE;
}
void DisableRegenHandler(){
    RegenButton = 0;
}
void DisableRegenDecider(){
    state = NORMAL_DRIVE;
}


//change name
const Tritium_state_t FSM[2] = {
    {NORMAL_DRIVE, &NormalDriveHandler, &NormalDriveDecider},
    {RECORD_VELOCITY, &RecordVelocityHandler, &RecordVelocityDecider},
    {POWERED_CRUISE, &PoweredCruiseHandler, &PoweredCruiseDecider},
    {COASTING_CRUISE, &CoastingCruiseHandler, &CoastingCruiseDecider},
    {BRAKE, &BrakeHandler, &BrakeDecider},
    {REGEN_DRIVE, &RegenDriveHandler, &RegenDriveDecider}
};

void Task_UpdateVelocity(void *p_arg){
    OS_ERR err;
    
    while(1){
        FSM[state].stateHandler();    // do what the current state does


        // Get inputs to decide actions
        accelPedalPercent = Pedals_Read(ACCELERATOR);
        brakePedalPercent = Pedals_Read(BRAKE);

        // Set brake lights
        if(brakePedalPercent <= UNTOUCH_PEDALS_PERCENT_BRAKE){
            Lights_Set(BrakeLight, ON);
        }
        else{
            Lights_Set(BrakeLight, OFF);
        }

        FSM[state].stateDecider();    // decide on the next state

        // Delay of few milliseconds (100)
        OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &err);
        if (err != OS_ERR_NONE){
            assertOSError(OS_UPDATE_VEL_LOC, err);
        }
    }
}