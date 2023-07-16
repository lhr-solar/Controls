/* Copyright (c) 2020 UT Longhorn Racing Solar */

/**
 * @defgroup Minions
 * 
 * This modules allows us to use GPIO more easily 
 * for our application's purposes
 * 
 */

/** @addtogroup Minions @{ */

#ifndef MINION_H
#define MINION_H
#include "common.h"
#include <stdbool.h>
#include "BSP_GPIO.h"

/* Should keep in line with the LUT in Minions.c */
#define FOREACH_PIN(PIN) \
        PIN(IGN_1)   \
        PIN(IGN_2)  \
        PIN(REGEN_SW)   \
        PIN(FOR_SW)  \
        PIN(REV_SW)  \
        PIN(CRUZ_EN)  \
        PIN(CRUZ_ST)  \
        PIN(BRAKELIGHT)  \

typedef enum MINIONPIN_ENUM {
    FOREACH_PIN(GENERATE_ENUM)
    NUM_PINS,
} pin_t;

typedef struct {
    uint16_t pinMask;
    port_t port;
    direction_t direction;
} pinInfo_t;

/**
 * @brief Initializes digital I/O
 * 
 */
void Minion_init(void);

/**
 * @brief Reads the status of a pin
 * 
 * @param pin 
 * @return true is high
 * @return false is low
 */
bool Minion_read(pin_t pin);

/**
 * @brief Updates the status of a pin
 * 
 * @param pin 
 * @param status 
 * @return true is fail (wrote to an input)
 * @return false is success (wrote to an output)
 */
bool Minion_write(pin_t pin, bool status);

#endif 

/** @} */