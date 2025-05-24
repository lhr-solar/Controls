#ifndef __DASHBOARD_H
#define __DASHBOARD_H

#include "daybreak_pins.h"
#include "BSP_GPIO.h"


typedef enum{
    SWITCH_ERROR = -1,
    SWITCH_OFF = 0,
    SWITCH_ON = 1,
    NUE,
    FWD,
    REV,
    NUM_SWITCH_STATES
}switch_state;

typedef enum{
    DASHBOARD_GEAR = 0,
    DASHBOARD_CRUZ_SET,
    NUM_DASH_PINS
}dashPin_t;

/**
 * @return switch_state Dashboard pin state
 * @param dashPin_t Dashboard pin
 */
 switch_state getDashState(dashPin_t pin);

/**
 * Initializes dashboard pins
 */
void dashboardInit();

#endif