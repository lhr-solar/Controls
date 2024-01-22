/**
 * @file Minions.h
 * @brief The Minions driver is responsible for most of the lights and 
 * switches in the system, and in particular those that are present on the Controls Lighting board. 
 * The driver uses GPIO to set/read pins on the MCU, which in turn are connected to the Lighting Board.
 * 
 */

#ifndef MINIONS_H
#define MINIONS_H

#include "BSP_GPIO.h"
#include "common.h"

// used to index into lookup table
// if changed, PINS_LOOKARR should be changed in Minions.c
#define FOREACH_PIN(PIN)                                             \
    PIN(kIgn1), PIN(kIgn2), PIN(kRegenSw), PIN(kForSw), PIN(kRevSw), \
        PIN(kCruzEn), PIN(kCruzSt), PIN(kBrakeLight),

/**
 * @brief Enumerates all software-controllable switches/lights
 * @details The enum is used to index into the lookup table PINS_LOOKARR in Minions.c
 * If the enum is changed, the lookup table should be changed accordingly.
*/
typedef enum {
    FOREACH_PIN(GENERATE_ENUM) kNumPins,
} Pin;

/**
 * @struct pinInfo_t
 * @brief Contains information about a pin
*/
typedef struct {
    /* GPIO Pin mask */
    uint16_t pin_mask;
    /* GPIO port */
    Port port;
    /* Direction (input/output)*/
    Direction direction;
} PinInfo;

/**
 * @brief Initializes digital I/O
 *
 */
void MinionsInit(void);

/**
 * @brief Reads the status of a pin
 * 
 * @param pin the pin to read
 * @return high or low
 */
bool MinionsRead(Pin pin);

/**
 * @brief Updates the status of a pin
 * 
 * @param pin the pint to read
 * @param status high or low
 * @return whether output was successfully written
 */
bool MinionsWrite(Pin pin, bool status);

#endif


