/* Copyright (c) 2020 UT Longhorn Racing Solar */

/**
 * @defgroup Minions
 *
 * This modules allows us to use GPIO more easily
 * for our application's purposes
 *
 * @defgroup Minions
 * @addtogroup Minions
 * @{
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

typedef enum {
    FOREACH_PIN(GENERATE_ENUM) kNumPins,
} Pin;

typedef struct {
    uint16_t pin_mask;
    Port port;
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
 * @param pin
 * @return true is high
 * @return false is low
 */
bool MinionsRead(Pin pin);

/**
 * @brief Updates the status of a pin
 *
 * @param pin
 * @param status
 * @return true is fail (wrote to an input)
 * @return false is success (wrote to an output)
 */
bool MinionsWrite(Pin pin, bool status);

#endif

/* @} */
