/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file SendTritium.h
 * @brief 
 * 
 * @defgroup SendTritium
 * @addtogroup SendTritium
 * @{
 */
#ifndef __SENDTRITIUM_MVP_CRUISE_H
#define __SENDTRITIUM_MVP_CRUISE_H

#include "common.h"

//#define SENDTRITIUM_PRINT_MES

#define MOTOR_MSG_PERIOD 100 // in ms
#define FSM_PERIOD 100 // in ms
#define DEBOUNCE_PERIOD 2 // in units of FSM_PERIOD
#define MOTOR_MSG_COUNTER_THRESHOLD (MOTOR_MSG_PERIOD)/(FSM_PERIOD)


// Macros
#define MAX_VELOCITY 20000.0f // rpm (unobtainable value)

#define MIN_CRUISE_VELOCITY mpsToRpm(20.0f)    // rpm
#define MAX_GEARSWITCH_VELOCITY mpsToRpm(8.0f) // rpm

#define BRAKE_PEDAL_THRESHOLD 50 // percent
#define ACCEL_PEDAL_THRESHOLD 15 // percent

#define PEDAL_MIN 0        // percent
#define PEDAL_MAX 100      // percent
#define CURRENT_SP_MIN 0   // percent
#define CURRENT_SP_MAX 100 // percent

#define GEAR_FAULT_THRESHOLD 3 // number of times gear fault can occur before it is considered a fault


#define FOREACH_Gear(GEAR) \
        GEAR(FORWARD_GEAR),   \
        GEAR(PARK_GEAR),  \
        GEAR(REVERSE_GEAR),   \

typedef enum GEAR_ENUM {
    FOREACH_Gear(GENERATE_ENUM)
    NUM_GEARS,
} Gear_t;

// State Names
typedef enum{
    FORWARD_DRIVE,
    PARK_STATE,
    REVERSE_DRIVE,
    RECORD_VELOCITY,
    POWERED_CRUISE,
    COASTING_CRUISE,
    ACCELERATE_CRUISE
} TritiumStateName_t;

// State Struct for FSM
typedef struct TritiumState{
    TritiumStateName_t name;
    void (*stateHandler)(void);
    void (*stateDecider)(void);
} TritiumState_t;

#ifdef SENDTRITIUM_EXPOSE_VARS
// Inputs
extern bool cruiseEnable;
extern bool cruiseSet;

extern uint8_t brakePedalPercent;
extern uint8_t accelPedalPercent;

extern Gear_t gear;

extern TritiumState_t state;
extern float velocityObserved;
extern float cruiseVelSetpoint;
#endif

// Getter functions for local variables in SendTritium.c
EXPOSE_GETTER(bool, cruiseEnable)
EXPOSE_GETTER(bool, cruiseSet)
EXPOSE_GETTER(uint8_t, brakePedalPercent)
EXPOSE_GETTER(uint8_t, accelPedalPercent)
EXPOSE_GETTER(Gear_t, gear)
EXPOSE_GETTER(TritiumState_t, state)
EXPOSE_GETTER(float, velocityObserved)
EXPOSE_GETTER(float, cruiseVelSetpoint)
EXPOSE_GETTER(float, currentSetpoint)
EXPOSE_GETTER(float, velocitySetpoint)

// Setter functions for local variables in SendTritium.c
#ifdef SENDTRITIUM_EXPOSE_VARS
EXPOSE_SETTER(bool, cruiseEnable)
EXPOSE_SETTER(bool, cruiseSet)
EXPOSE_SETTER(uint8_t, brakePedalPercent)
EXPOSE_SETTER(uint8_t, accelPedalPercent)
EXPOSE_SETTER(Gear_t, gear)
EXPOSE_SETTER(TritiumState_t, state)
EXPOSE_SETTER(float, velocityObserved)
EXPOSE_SETTER(float, cruiseVelSetpoint)
EXPOSE_SETTER(float, currentSetpoint)
EXPOSE_SETTER(float, velocitySetpoint)
#endif

#endif


/* @} */
