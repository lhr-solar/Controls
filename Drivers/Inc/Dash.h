#ifndef __DASH_H
#define __DASH_H

#include "daybreak_pins.h"
#include "BSP_GPIO.h"

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
    NUE = 0,
    FWD = 1,
    REV = 2,
    NUM_SWITCH_STATES
}switch_state;

typedef enum{
    OFF_IGN = 0,
    IGN_1,
    IGN_2,
    NUM_IGN_STATES
}ign_state;

typedef enum{
    BPS = 0,
    BRAKE_LED,
    GEAR,
    HBT,
    CRUZ_SET,
    CRUZ_EN,
    IGN,
    NUM_DASH_PINS
}dashPin_t;

/**
 * @return Dashboard pin state (0 low, 1 high)
 * @param dashPin_t Dashboard pin
 */
uint8_t getDash(dashPin_t pin);

/**
 * Initializes dashboard pins
 */
void dashInit();

#endif