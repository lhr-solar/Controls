/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file StatusLeds.h
 * @brief Header file for the Lights driver
 */

#ifndef __LIGHTS_H
#define __LIGHTS_H

#include "common.h"
#include "BSP_GPIO.h"

typedef enum {
    RIGHT_BLINK_LIGHT,
    LEFT_BLINK_LIGHT,
    BRAKE_LIGHT,
    NUM_LIGHTS,
} light_t;
 
/**
 * @brief Initializes Light pins
 * 
 */
void Lights_Init(void);

/**
 * @brief   Writes data to a specified pin
 * @param   led The led to write to
 * @param   state true=ON or false=OFF
 * @return  None
 */ 
void Lights_Write(light_t led, bool state);

#endif