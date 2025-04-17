#ifndef __DASH_H
#define __DASH_H

#include "daybreak_pins.h"
#include "BSP_GPIO.h"

typedef struct{
    uint8_t BPS_HZD;
    uint8_t BRAKE;
    uint8_t GEAR;
    uint8_t HBT;
    uint8_t CRUZ_SET;
    uint8_t CRUZ_EN;
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
 * input: dashboard pin
 * @return dashboard pin state
 */
uint8_t getDash(dashPin_t pin);

/**
 * inits dash pins
 */
void dashInit();

/**
 * updates dash pins
 */

#endif