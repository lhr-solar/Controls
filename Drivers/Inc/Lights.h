/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file StatusLeds.h
 * @brief Header file for the StatusLeds driver
 */

#ifndef __LIGHTS_H
#define __LIGHTS_H

#include "common.h"


/**
 * Possible Lights
 */
typedef enum {
    RIGHT_LIGHT,
    LEFT_LIGHT,
    NUM_LIGHTS,
} lights_t;
 
/**
 * @brief Initializes Status LED pins
 * 
 */
void Lights_Init(void);

/**
 * @brief   Writes data to a specified pin
 * @param   led The led to write to
 * @param   state true=ON or false=OFF
 * @return  None
 */ 
void Lights_Write(lights_t led, uint8_t duty);

/**
 * @brief   Toggles a status led
 * @param   led The led to toggle
 * @return  None
 */ 
void Lights_All_On(void);

/**
 * @brief   Turns all status LEDs off
 * @return  None
 */ 
void Lights_All_Off(void);


#endif
