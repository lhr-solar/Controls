/* Copyright (c) 2020 UT Longhorn Racing Solar */

#ifndef __CAR_STATE_H
#define __CAR_STATE_H

#include "Switches.h"
#include "common.h"
#include "config.h"



/**
 * Car State
 * 
 * Stores all relevant data about the current
 * state of the car
 * This is used by all the tasks to communicate
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

typedef struct {
    float DesiredVelocity;
    float CruiseControlVelocity;
    float CurrentVelocity;
    float DesiredMotorCurrent;

    uint8_t AccelPedalPercent;
    uint8_t BrakePedalPercent;
    uint8_t MotorCurrentSetpoint;

    switch_states_t SwitchStates;

    State CruiseControlEnable;
    State CruiseControlSet;

    State ShouldArrayBeActivated;
    State ShouldMotorBeActivated;

    State IsRegenBrakingAllowed;

    error_code_t ErrorCode;

} car_state_t;

#endif
