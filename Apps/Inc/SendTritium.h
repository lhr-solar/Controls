#ifndef __SENDTRITIUM_H
#define __SENDTRITIUM_H

#define MOTOR_MSG_PERIOD 100
#define FSM_PERIOD 20
#define DEBOUNCE_PERIOD 2 // in units of FSM_PERIOD
#define MOTOR_MSG_COUNTER_THRESHOLD (MOTOR_MSG_PERIOD)/(FSM_PERIOD)

#ifdef __TEST_SENDTRITIUM
// Inputs
extern bool cruiseEnable;
extern bool cruiseSet;
extern bool onePedalEnable;
extern bool chargeEnable;

extern uint8_t brakePedalPercent;
extern uint8_t accelPedalPercent;

extern bool forwardGear;
extern bool neutralGear;
extern bool reverseGear;

extern uint8_t state;
extern float velocityObserved;
extern float cruiseVelSetpoint;
#endif

#endif
