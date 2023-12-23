/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file SendTritium.h
 * @brief SendTritium contains functions relevant to updating the velocity and current setpoitns
 * of the Tritium motor controller. The implementation includes a normal current
 * controlled mode, a one-pedal driving mode (with regenerative braking), and cruise control.
 * The logic is determined through a finite state machine implementation.
 */
#ifndef __SENDTRITIUM_H
#define __SENDTRITIUM_H

#include "common.h"

/**
 * @def SENDTRITIUM_PRINT_MES
 * @brief Print messages for SendTritium.c
 */
// #define SENDTRITIUM_PRINT_MES

/**
 * @def SENDTRITIUM_EXPOSE_VARS
 * @brief Expose variables for SendTritium.c
 */
// #define SENDTRITIUM_EXPOSE_VARS

/**
 * @def MOTOR_MSG_PERIOD
 * @brief Period of the motor CAN message in milliseconds
 */
#define MOTOR_MSG_PERIOD 100

/**
 * @def FSM_PERIOD
 * @brief Period of the state machine in milliseconds
 */
#define FSM_PERIOD 100

/**
 * @def DEBOUNCE_PERIOD
 * @brief Period of the debouncer in units of FSM_PERIOD
 */
#define DEBOUNCE_PERIOD 2

/**
 * Generate enum list for Gear_t
*/
#define FOREACH_Gear(GEAR) \
        GEAR(FORWARD_GEAR),   \
        GEAR(NEUTRAL_GEAR),  \
        GEAR(REVERSE_GEAR),   \

/**
 * @enum Gear_t
 * @brief Enumerated list of gears
*/
typedef enum GEAR_ENUM {
    FOREACH_Gear(GENERATE_ENUM)
    NUM_GEARS,
} Gear_t;

/**
 * @enum TritiumStateName_t
 * @brief Enumerated list of states for the FSM
*/
typedef enum{
    /** Forward drive state */
    FORWARD_DRIVE,   
    /** Neutral drive state */
    NEUTRAL_DRIVE,   
    /** Reverse drive state */
    REVERSE_DRIVE,   
    /** Record velocity state (cruise) */
    RECORD_VELOCITY, 
    /** Powered cruise state (cruise) */
    POWERED_CRUISE,  
    /** Coasting cruise state (cruise) */
    COASTING_CRUISE, 
    /** Brake state */
    BRAKE_STATE,     
    /** One pedal state (regen) */
    ONEPEDAL,        
    /** Accelerate cruise state (cruise) */
    ACCELERATE_CRUISE   
} TritiumStateName_t;

/**
 * @struct TritiumState_t
 * @brief Struct containing the state name, state handler, and state decider
 * function for the Tritium FSM
*/
typedef struct TritiumState{
    /** Name of the state */
    TritiumStateName_t name;   
    /** Function pointer to the state handler */ 
    void (*stateHandler)(void);
    /** Function pointer to the state decider */ 
    void (*stateDecider)(void); 
} TritiumState_t;

#ifdef SENDTRITIUM_EXPOSE_VARS
// Inputs
extern bool cruiseEnable;
extern bool cruiseSet;
extern bool onePedalEnable;
extern bool regenEnable;

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
EXPOSE_GETTER(bool, onePedalEnable)
EXPOSE_GETTER(bool, regenEnable)
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
EXPOSE_SETTER(bool, onePedalEnable)
EXPOSE_SETTER(bool, regenEnable)
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



