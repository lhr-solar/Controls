/* Copyright (c) 2020 UT Longhorn Racing Solar */

#ifndef __CAR_STATE_H
#define __CAR_STATE_H

#include "common.h"
#include "config.h"

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

#define SYSTEM_ERR_COUNT (sizeof(error_code_t) / sizeof(State))

union error_array_t {
    error_code_t err;
    State arr[SYSTEM_ERR_COUNT];
};


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

    blinker_states_t BlinkerStates;

    CruiseRegenSet CRSet;
    State CruiseControlEnable;
    
    int RegenBrakeRate;
    State IsRegenBrakingAllowed;
    RegenMode RegenButtonMode;

    State ShouldArrayBeActivated;
    State ShouldMotorBeActivated;

    error_code_t ErrorCode;
    
} car_state_t;


#endif
