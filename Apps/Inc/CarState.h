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
    T_NONE = 0x00,
    T_TEMP_ERR = 0x01,
    T_CC_VEL_ERR = 0x02,
    T_SLIP_SPEED_ERR = 0x04,
    T_OVER_SPEED_ERR = 0x08
} tritium_error_code_t;

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
    OS_NONE_LOC = 0x000,
    OS_ARRAY_LOC = 0x001,
    OS_READ_CAN_LOC = 0x002,
    OS_READ_TRITIUM_LOC = 0x004,
    OS_SEND_CAN_LOC = 0x008,
    OS_SEND_TRITIUM_LOC = 0x010,
    OS_UPDATE_VEL_LOC = 0x020,
    OS_READ_PEDAL_LOC = 0x040,
    OS_BLINK_LIGHTS_LOC = 0x080,
    OS_MOTOR_CONNECTION_LOC = 0x100
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

    os_error_loc_t OSErrorLocBitmap;
    tritium_error_code_t TritiumErrorBitmap;
    fault_bitmap_t FaultBitmap;

} car_state_t;


#endif
