/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////
#ifdef TEST_SENDTRITIUM
#include_next "SendTritium.h"
#else

#ifndef __SENDTRITIUM_H
#define __SENDTRITIUM_H

#include "fff.h"
#include "common.h"

//#define SENDTRITIUM_PRINT_MES

#define MOTOR_MSG_PERIOD 100 // in ms
#define FSM_PERIOD 100 // in ms
#define DEBOUNCE_PERIOD 2 // in units of FSM_PERIOD
#define MOTOR_MSG_COUNTER_THRESHOLD (MOTOR_MSG_PERIOD)/(FSM_PERIOD)

#define FOREACH_Gear(GEAR) \
        GEAR(FORWARD_GEAR),   \
        GEAR(NEUTRAL_GEAR),  \
        GEAR(REVERSE_GEAR),   \

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


DECLARE_FAKE_VOID_FUNC(Task_SendTritium, void*);

// Getter functions for local variables in SendTritium.c
DECLARE_FAKE_VALUE_FUNC(bool, GetCruiseEnable);

DECLARE_FAKE_VALUE_FUNC(bool, GetCruiseSet);

DECLARE_FAKE_VALUE_FUNC(bool, GetOnePedalEnable);

DECLARE_FAKE_VALUE_FUNC(bool, GetRegenEnable);

DECLARE_FAKE_VALUE_FUNC(uint8_t, GetBrakePedalPercent);

DECLARE_FAKE_VALUE_FUNC(uint8_t, GetAccelPedalPercent);

DECLARE_FAKE_VALUE_FUNC(Gear_t, GetGear);

DECLARE_FAKE_VALUE_FUNC(TritiumState_t, GetState);

DECLARE_FAKE_VALUE_FUNC(float, GetVelocityObserved);

DECLARE_FAKE_VALUE_FUNC(float, GetCruiseVelSetpoint);

DECLARE_FAKE_VALUE_FUNC(float, GetCurrentSetpoint);

DECLARE_FAKE_VALUE_FUNC(float, GetVelocitySetpoint);


// Setter functions for local variables in SendTritium.c
#ifdef SENDTRITIUM_EXPOSE_VARS
DECLARE_FAKE_VOID_FUNC(SetCruiseEnable, bool);

DECLARE_FAKE_VOID_FUNC(SetCruiseSet, bool);

DECLARE_FAKE_VOID_FUNC(SetOnePedalEnable, bool);

DECLARE_FAKE_VOID_FUNC(SetRegenEnable, bool);

DECLARE_FAKE_VOID_FUNC(SetBrakePedalPercent, uint8_t);

DECLARE_FAKE_VOID_FUNC(SetAccelPedalPercent, uint8_t);

DECLARE_FAKE_VOID_FUNC(SetGear, Gear_t);

DECLARE_FAKE_VOID_FUNC(SetState, TritiumState_t);

DECLARE_FAKE_VOID_FUNC(SetVelocityObserved, float);

DECLARE_FAKE_VOID_FUNC(SetCruiseVelSetpoint, float);

DECLARE_FAKE_VOID_FUNC(SetCurrentSetpoint, float);

DECLARE_FAKE_VOID_FUNC(SetVelocitySetpoint, float);
#endif

#endif
#endif