/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file SendTritium.h
 * @brief
 *
 * @defgroup SendTritium
 * @addtogroup SendTritium
 * @{
 */
#ifndef SENDTRITIUM_H
#define SENDTRITIUM_H

#include "common.h"

// #define SENDTRITIUM_PRINT_MES

#define MOTOR_MSG_PERIOD 100
#define FSM_PERIOD 100
#define DEBOUNCE_PERIOD 2  // in units of FSM_PERIOD
#define MOTOR_MSG_COUNTER_THRESHOLD ((MOTOR_MSG_PERIOD) / (FSM_PERIOD))

#define FOREACH_GEAR(GEAR) \
    GEAR(kForwardGear), GEAR(kNeutralGear), GEAR(kReverseGear),

typedef enum {
    FOREACH_GEAR(GENERATE_ENUM) kNumGears,
} Gear;

// State Names
typedef enum {
    kForwardDrive,
    kNeutralDrive,
    kReverseDrive,
    kRecordVelocity,
    kPoweredCruise,
    kCoastingCruise,
    kBrakeState,
    kOnepedal,
    kAccelerateCruise
} TritiumStateName;

// State Struct for FSM
typedef struct TritiumState {
    TritiumStateName name;
    void (*stateHandler)(void);
    void (*stateDecider)(void);
} TritiumState;

// Getter functions for static variables
bool GetCruiseEnable(void);
bool GetCruiseSet(void);
bool GetOnePedalEnable(void);
bool GetRegenEnable(void);
uint8_t GetBrakePedelPercent(void);
uint8_t GetAccelPedelPercent(void);
Gear GetGear(void);
TritiumState GetState(void);
float GetVelocityObserved(void);
float GetCruiseVelSetpoint(void);
float GetCurrentSetpoint(void);
float GetVelocitySetpoint(void);

#ifdef SENDTRITIUM_EXPOSE_VARS
// Setter functions for static variables
void SetCruiseEnable(bool value);
void SetCruiseSet(bool value);
void SetOnePedalEnable(bool value);
void SetRegenEnable(bool value);
void SetBrakePedelPercent(uint8_t value);
void SetAccelPedelPercent(uint8_t value);
void SetGear(Gear value);
void SetState(TritiumState value);
void SetVelocityObserved(float value);
void SetCruiseVelSetpoint(float value);
void SetCurrentSetpoint(float value);
void SetVelocitySetpoint(float value);
#endif

#endif

/* @} */
