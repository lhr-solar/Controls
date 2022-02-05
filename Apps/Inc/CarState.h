/* Copyright (c) 2020 UT Longhorn Racing Solar */

#ifndef __CAR_STATE_H
#define __CAR_STATE_H

#include "common.h"
#include "config.h"


/**
 * Motor Error States
 * Read messages from motor in ReadTritium and trigger appropriate error messages as needed based on bits
 * 
 */

typedef struct{
    State motorTempErr;
    State CCVelocityErr; 
    State slipSpeedErr;
    State overSpeedErr;
} motor_error_code_t;

/**
 * Switch States
 * 
 * Stores the current state of each of
 * the switches that control this system
 */
typedef struct {
    State LT;
    State RT;
    State FWD;
    State REV;
    State CRS_EN;
    State CRS_SET;
    State REGEN;
    State HZD;
    State HDLT;
    State IGN_1;
    State IGN_2;
} switch_states_t;

/**
 * Blinker States
 * 
 * Stores the desired blinker state
 * to indicate which lights to toggle
 */
typedef struct {
    State HZD;
    State LT;
    State RT;
} blinker_states_t;

/**
 * Error States
 * 
 * Stores error data to indicate which part of the code
 * an error is coming from.
 */

typedef struct{
    State ArrayErr;
    State ReadCANErr;
    State ReadTritiumErr;
    State SendCANErr;
    State SendTritiumErr;
    State UpdateVelocityErr;
    State ReadPedalErr;
    State BlinkLightsErr;
    State MotorConnectionErr;
} error_code_t;


typedef struct{
    State Fault_OS : 1;         // for OS faults
    State Fault_UNREACH : 1;    // for unreachable conditions
    State Fault_TRITIUM : 1;      // for errors sent from the tritium
    State Fault_READBPS : 1;    // for unsuccessfully reading from BPS CAN
    State Fault_DISPLAY : 1;    // for display faults
} fault_conditions_t;
/**
 * Regen Brake Mode Enum
 * 
 * Different modes of the regenerative braking
 * button
 */
typedef enum{REGEN_OFF, RATE3, RATE2, RATE1} RegenMode;

/**
 * Regen/Cruise Set Enum
 */

typedef enum{ACCEL, REGEN, CRUISE} CruiseRegenSet;


/**
 * Car State
 * 
 * Stores all relevant data about the current
 * state of the car
 * This is used by all the tasks to communicate
 */

typedef struct {
    float DesiredVelocity;
    float CruiseControlVelocity;
    float CurrentVelocity;
    float DesiredMotorCurrent;

    uint8_t AccelPedalPercent;
    uint8_t BrakePedalPercent;
    uint8_t MotorCurrentSetpoint;

    switch_states_t SwitchStates;
    blinker_states_t BlinkerStates;

    CruiseRegenSet CRSet;
    State CruiseControlEnable;
    
    int RegenBrakeRate;
    State IsRegenBrakingAllowed;
    RegenMode RegenButtonMode;

    State ShouldArrayBeActivated;
    State ShouldMotorBeActivated;

    error_code_t ErrorCode;
    motor_error_code_t MotorErrorCode;
    fault_conditions_t FaultConditions;

} car_state_t;


#endif
