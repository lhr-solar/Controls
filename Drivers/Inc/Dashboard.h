#ifndef __DASHBOARD_H
#define __DASHBOARD_H

#include "daybreak_pins.h"
#include "BSP_GPIO.h"

// Dasboard leds are negative logic
#define DASHBOARD_LEDS_ON 0
#define DASHBOARD_LEDS_OFF 1

typedef struct{
    bool BPS_HZD;
    bool BRAKE;
    uint8_t GEAR;
    bool HBT;
    bool CRUZ_SET;
    bool CRUZ_EN;
    uint8_t IGN;
}dashboard_t;


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
    BPS = 0,
    BRAKE_LED,
    GEAR,
    HBT,
    CRUZ_SET,
    CRUZ_EN,
    NUM_DASH_PINS
}dashPin_t;

/**
 * @return Dashboard pin state
 * @param dashPin_t Dashboard pin
 */
 switch_state getDashState(dashPin_t pin);

/**
 * Initializes dashboard pins
 */
void dashboardInit();

#endif