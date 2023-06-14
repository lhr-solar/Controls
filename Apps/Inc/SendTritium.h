#ifndef __SENDTRITIUM_H
#define __SENDTRITIUM_H

#include "getter.h"

#define MOTOR_MSG_PERIOD 500
#define FSM_PERIOD 250
#define DEBOUNCE_PERIOD 2 // in units of FSM_PERIOD
#define MOTOR_MSG_COUNTER_THRESHOLD (MOTOR_MSG_PERIOD)/(FSM_PERIOD)

#define FOREACH_Gear(GEAR) \
        GEAR(FORWARD_GEAR)   \
        GEAR(NEUTRAL_GEAR)  \
        GEAR(REVERSE_GEAR)   \

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

typedef enum GEAR_ENUM {
    FOREACH_Gear(GENERATE_ENUM)
    NUM_GEARS,
} Gear_t;

// State Names
typedef enum{
    FORWARD_DRIVE,
    NEUTRAL_DRIVE,
    REVERSE_DRIVE,
    RECORD_VELOCITY,
    POWERED_CRUISE,
    COASTING_CRUISE,
    BRAKE_STATE,
    ONEPEDAL,
    ACCELERATE_CRUISE
} TritiumStateName_t;

// State Struct for FSM
typedef struct TritiumState{
    TritiumStateName_t name;
    void (*stateHandler)(void);
    void (*stateDecider)(void);
} TritiumState_t;

#define __TEST_SENDTRITIUM
#ifdef __TEST_SENDTRITIUM
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

// Getters
GETTER_H(bool, cruiseEnable)
GETTER_H(bool, cruiseSet)
GETTER_H(bool, onePedalEnable)
GETTER_H(bool, regenEnable)
GETTER_H(uint8_t, brakePedalPercent)
GETTER_H(uint8_t, accelPedalPercent)
GETTER_H(Gear_t, gear)
GETTER_H(float, currentSetpoint)
GETTER_H(float, velocitySetpoint)
GETTER_H(float, cruiseVelSetpoint)
GETTER_H(float, velocityObserved)

#endif
