/**
 * @file SendTritium.h
 * @brief Runs the car's State Machine and sends control messages to the Tritium motor controller. 
 * 
 * Starting the SendTritium task will start sending CAN messages 
 * and running the state machine. For details on the state machine, see
 * 
 */
#ifndef SENDTRITIUM_H
#define SENDTRITIUM_H

#include "common.h"

/**
 * Print messages for SendTritium.c
 */
// #define SENDTRITIUM_PRINT_MES

/**
 * Expose variables for SendTritium.c
 */
// #define SENDTRITIUM_EXPOSE_VARS

/**
 * Expose variables for SendTritium.c
 */
// #define SENDTRITIUM_EXPOSE_VARS

/**
 * Period of the motor CAN message in milliseconds
 */
#define MOTOR_MSG_PERIOD 100

/**
 * Period of the state machine in milliseconds
 */
#define FSM_PERIOD 100

/**
 * Period of the debouncer in units of FSM_PERIOD
 */
#define DEBOUNCE_PERIOD 2

/**
 * Generate enum list for Gear_t
*/
#define FOREACH_GEAR(GEAR) \
    GEAR(kForwardGear), GEAR(kNeutralGear), GEAR(kReverseGear),

/**
 * Gears
*/
typedef enum {
    FOREACH_GEAR(GENERATE_ENUM) kNumGears,
} Gear;

/**
 * States for the FSM
*/
typedef enum {
    /** Forward drive state */
    kForwardDrive,   
    /** Neutral drive state */
    kNeutralDrive,   
    /** Reverse drive state */
    kReverseDrive,   
    /** Record velocity state (cruise) */
    kRecordVelocity, 
    /** Powered cruise state (cruise) */
    kPoweredCruise,  
    /** Coasting cruise state (cruise) */
    kCoastingCruise, 
    /** Brake state */
    kBrakeState,     
    /** One pedal state (regen) */
    kOnePedal,        
    /** Accelerate cruise state (cruise) */
    kAccelerateCruise   
} TritiumStateName;

/**
 * Struct containing the state name, state handler, and state decider
 * function for the Tritium FSM
*/
typedef struct TritiumState {
    /** Name of the state */
    TritiumStateName name;   
    /** Function pointer to the state handler */ 
    void (*stateHandler)(void);
    /** Function pointer to the state decider */ 
    void (*stateDecider)(void); 
} TritiumState;

// Getter functions for static variables
bool GetCruiseEnable(void);
bool GetCruiseSet(void);
bool GetOnePedalEnable(void);
bool GetRegenEnable(void);
uint8_t GetBrakePedalPercent(void);
uint8_t GetAccelPedalPercent(void);
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
void SetBrakePedalPercent(uint8_t value);
void SetAccelPedalPercent(uint8_t value);
void SetGear(Gear value);
void SetState(TritiumState value);
void SetVelocityObserved(float value);
void SetCruiseVelSetpoint(float value);
void SetCurrentSetpoint(float value);
void SetVelocitySetpoint(float value);
#endif

#endif
