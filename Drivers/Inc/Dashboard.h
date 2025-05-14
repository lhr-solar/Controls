#ifndef __DASHBOARD_H
#define __DASHBOARD_H

#include "daybreak_pins.h"
#include "BSP_GPIO.h"

#define FOREACH_Gear(GEAR) \
        GEAR(NEU), \
        GEAR(FWD), \
        GEAR(REV), \

typedef enum {
    GEAR_FAULT_ERROR = -1,
    FOREACH_Gear(GENERATE_ENUM)
    NUM_GEARS,
} gear_t;

typedef enum{
    SWITCH_ERROR = -1,
    SWITCH_OFF,
    SWITCH_ON,
    NUM_SWITCH_STATES,
} switch_state_t;

typedef enum{
    //BPS = 0,
    //BRAKE_LED,
    // GEAR = 0,
    //HBT,
    CRUZ_SET,
    CRUZ_EN,
    NUM_DASH_PINS
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