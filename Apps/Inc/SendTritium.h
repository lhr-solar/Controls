#ifndef __SENDTRITIUM_H
#define __SENDTRITIUM_H

//#define __TEST_SENDTRITIUM

#ifdef __TEST_SENDTRITIUM
// Inputs
extern bool cruiseEnable;
extern bool cruiseSet;
extern bool onePedalEnable;
extern bool chargeEnable;
extern uint8_t brakePedalPercent;
extern uint8_t accelPedalPercent;
extern bool reverseSwitch;
extern bool forwardSwitch;
extern uint8_t state;
extern float currentSetpoint;
extern float velocitySetpoint;
extern float velocityObserved;
extern float cruiseVelSetpoint;
#endif

#endif
