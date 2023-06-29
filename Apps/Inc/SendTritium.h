#ifndef __SENDTRITIUM_H
#define __SENDTRITIUM_H

/** @defgroup SendTritium */
/** @ingroup SendTritium 
 * @{ */

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

/*@}*/
