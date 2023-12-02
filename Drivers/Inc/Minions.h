/* Copyright (c) 2020 UT Longhorn Racing Solar */

/**
 * 
 * This modules allows us to use GPIO more easily 
 * for our application's purposes
 * 
 * @addtogroup Minions
 * @{
 */

#ifndef MINIONS_H
#define MINIONS_H
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

/** @} */
