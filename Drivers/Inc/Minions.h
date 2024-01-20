/**
 * @file Minions.h
 * @brief The Minions driver is responsible for most of the lights and 
 * switches in the system, and in particular those that are present on the Controls Lighting board. 
 * The driver uses GPIO to set/read pins on the MCU, which in turn are connected to the Lighting Board.
 * 
 */

#ifndef MINIONS_H
#define MINIONS_H
#include "common.h"
#include <stdbool.h>
#include "BSP_GPIO.h"

// used to index into lookup table
// if changed, PINS_LOOKARR should be changed in Minions.c
#define FOREACH_PIN(PIN) \
        PIN(IGN_1),   \
        PIN(IGN_2),  \
        PIN(REGEN_SW),   \
        PIN(FOR_SW),  \
        PIN(REV_SW),  \
        PIN(CRUZ_EN),  \
        PIN(CRUZ_ST),  \
        PIN(BRAKELIGHT),  \

/**
 * @brief Enumerates all software-controllable switches/lights
 * @details The enum is used to index into the lookup table PINS_LOOKARR in Minions.c
 * If the enum is changed, the lookup table should be changed accordingly.
*/
typedef enum MINIONPIN_ENUM {
    FOREACH_PIN(GENERATE_ENUM)
    NUM_PINS,
} pin_t;

/**
 * @struct pinInfo_t
 * @brief Contains information about a pin
*/
typedef struct {
    /* GPIO Pin mask */
    uint16_t pinMask;
    /* GPIO port */
    port_t port;
    /* Direction (input/output)*/
    direction_t direction;
} pinInfo_t;

/**
 * @brief Initializes digital I/O
 * 
 */
void Minions_Init(void);

/**
 * @brief Reads the status of a pin
 * 
 * @param pin the pin to read
 * @return high or low
 */
bool Minions_Read(pin_t pin);

/**
 * @brief Updates the status of a pin
 * 
 * @param pin the pint to read
 * @param status high or low
 * @return whether output was successfully written
 */
bool Minions_Write(pin_t pin, bool status);

#endif 


