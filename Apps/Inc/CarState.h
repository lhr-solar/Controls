/* Copyright (c) 2020 UT Longhorn Racing Solar */

#ifndef __CAR_STATE_H
#define __CAR_STATE_H

#include "common.h"
#include "config.h"

/**
 * Switch States
 * 
 * Stores the current state of each of
 * the switches that control this system
 */

typedef struct switch_states_t {
    State CRUZ_SW;
    State CRUZ_EN;
    State HZD_SQ;
    State FR_SW;
    State HEADLIGHT_SW;
    State LEFT_SW;
    State RIGHT_SW;
    State REGEN_SW;
    State IGN_1;
    State IGN_2;
    State REV_SW;
}switch_states_t;


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
 * Car State
 * 
 * Stores all relevant data about the current
 * state of the car
 * This is used by all the tasks to communicate
 */
typedef struct {
    uint8_t DesiredVelocity;
    uint8_t CruiseControlVelocity;
    uint8_t CurrentVelocity;

    uint8_t AccelPedalPercent;
    uint8_t BrakePedalPercent;
    uint8_t MotorCurrentSetpoint;

    switch_states_t SwitchStates;
    blinker_states_t BlinkerStates;

    State CruiseControlEnable;
    State CruiseControlSet;
} car_state_t;

#endif
