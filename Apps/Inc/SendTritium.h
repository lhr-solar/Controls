#ifndef __SENDTRITIUM_H
#define __SENDTRITIUM_H

#define MOTOR_MSG_PERIOD 500
#define FSM_PERIOD 250
#define DEBOUNCE_PERIOD 2 // in units of FSM_PERIOD
#define MOTOR_MSG_COUNTER_THRESHOLD (MOTOR_MSG_PERIOD)/(FSM_PERIOD)

typedef enum{
	FORWARD_GEAR,
	NEUTRAL_GEAR,
	REVERSE_GEAR
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

/* TODO: LET NATHANIEL FINISH THIS

// used to index into lookup table
// if changed, PINS_LOOKARR should be changed in Minions.c
#define FOREACH_CurrentState(STATE) \
        STATE(FORWARD_DRIVE)   \
        STATE(NEUTRAL_DRIVE)  \
        PIN(REVERSE_DRIVE)   \
        PIN(RECORD_VELOCITY)  \
        PIN(POWERED_CRUISE)  \
        PIN(COASTING_CRUISE)  \
        PIN(BRAKE_STATE)  \
        PIN(ONEPEDAL)  \
        PIN(ACCELERATE_CRUISE)  \


#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

typedef enum MINIONPIN_ENUM {
    FOREACH_MinionPin(GENERATE_ENUM)
    MINIONPIN_NUM,
}TritiumStateName_t;

*/

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

#endif
