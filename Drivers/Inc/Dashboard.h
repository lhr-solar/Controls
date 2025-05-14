#ifndef __DASHBOARD_H
#define __DASHBOARD_H

#include "daybreak_pins.h"
#include "BSP_GPIO.h"

#define FOREACH_Gear(GEAR) \
        GEAR(DASH_NEU), \
        GEAR(DASH_FWD), \
        GEAR(DASH_REV), \

typedef enum {
    DASH_GEAR_FAULT_ERROR = -1,
    FOREACH_Gear(GENERATE_ENUM)
    DASH_NUM_GEARS,
} gear_t;

typedef enum{
    DASH_SWITCH_ERROR = -1,
    DASH_SWITCH_OFF,
    DASH_SWITCH_ON,
    DASH_NUM_SWITCH_STATES,
} switch_state_t;

typedef enum{
    //BPS = 0,
    //BRAKE_LED,
    // GEAR = 0,
    //HBT,
    DASH_CRUZ_SET,
    DASH_CRUZ_EN,
    DASH_NUM_PINS,
} dash_pin_t;

/**
 * @brief Retrieves the current gear of the car
 * @return Returns the current car gear
 */
gear_t getGear(void);

/**
 * @return switch_state Dashboard pin state
 * @param dashPin_t Dashboard pin
 */
 switch_state_t getSwitchState(dash_pin_t pin);

/**
 * Initializes dashboard pins
 */
void dashboardInit();

#endif