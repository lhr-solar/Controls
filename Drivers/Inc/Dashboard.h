#ifndef __DASHBOARD_H
#define __DASHBOARD_H

#include "daybreak_pins.h"
#include "BSP_GPIO.h"

#define NEUTRAL_DEBOUNCE_COUNT 5
#define NEUTRAL_DEBOUNCE_DLY_MS 10

#define FOREACH_Gear(GEAR) \
        GEAR(DASH_NEU), \
        GEAR(DASH_FWD), \
        GEAR(DASH_REV), \

typedef enum {
    DASH_GEAR_FAULT_ERROR = -1,
    FOREACH_Gear(GENERATE_ENUM)
    DASH_NUM_GEARS,
} gear_t;

// NOTE: Relevant when we're using cruise
typedef enum{
    DASH_SW_ERROR = -1,
    DASH_SW_OFF,
    DASH_SW_ON,
    DASH_NUM_SW_STATES,
} switch_state_t;

typedef enum{
    DASH_CRUZ_SET,
    DASH_CRUZ_EN,
    DASH_RIGHT_IND,
    DASH_LEFT_IND,
    DASH_HZD,
    DASH_NUM_PINS,
} dash_pin_t;

#define GEAR_USE_OS_DELAY true


/**
 * @brief Retrieves the current gear of the car
 * @param useOSDelay If true, uses OS delay for debouncing; otherwise, uses assembly delay
 * @return Returns the current car gear
 */
gear_t getGear(bool useOSDelay);

/**
 * @return switch_state Dashboard pin state
 * @param dashPin_t Dashboard pin
 */
// NOTE: Relevant when we're using cruise
switch_state_t getSwitchState(dash_pin_t pin);

/**
 * Initializes dashboard pins
 */
void dashboardInit();

#endif