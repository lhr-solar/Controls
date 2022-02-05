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

typedef enum{
    M_NONE = 0x00,
    M_TEMP_ERR = 0x01,
    M_CC_VEL_ERR = 0x02,
    M_SLIP_SPEED_ERR = 0x04,
    M_OVER_SPEED_ERR = 0x08
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
 * OS Error States
 * 
 * Stores error data to indicate which part of the code
 * an error is coming from.
 */

typedef enum{
    ARRAY_ERR = 0x001,
    READ_CAN_ERR = 0x002,
    READ_TRITIUM_ERR = 0x004,
    SEND_CAN_ERR = 0x008,
    SEND_TRITIUM_ERR = 0x010,
    UPDATE_VEL_ERR = 0x020,
    READ_PEDAL_ERR = 0x040,
    BLINK_LIGHTS_ERR = 0x080,
    MOTOR_CONNECTION_ERR = 0x100
} os_error_loc_t;

/**
 * Fault Union
 * 
 * Different fault states that need to be handled by the FaultState task
 */
typedef union{
    uint8_t bitmap;
    struct{
        State Fault_OS : 1;         // for OS faults
        State Fault_UNREACH : 1;    // for unreachable conditions
        State Fault_TRITIUM : 1;      // for errors sent from the tritium
        State Fault_READBPS : 1;    // for unsuccessfully reading from BPS CAN
        State Fault_DISPLAY : 1;    // for display faults
    };
} fault_bitmap_t;

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

    uint8_t OSErrorLocBitmap;
    uint16_t MotorErrorBitmap;
    fault_bitmap_t FaultBitmap;

} car_state_t;


#endif
