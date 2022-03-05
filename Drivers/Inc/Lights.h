/* Copyright (c) 2020 UT Longhorn Racing Solar */

#ifndef __Lights_H
#define __Lights_H

#include "BSP_GPIO.h"
#include "BSP_SPI.h"
#include "common.h"
#include "config.h"
#include "GPIOExpander.h"

#define LIGHTS_PORT     PORTC
#define HEADLIGHT_PIN   6
#define BRAKELIGHT_PIN  7
#define RIGHT_BLINK_PIN 8
#define LEFT_BLINK_PIN  9

// Ordered by pin of minion board
//TODO: Double check that this ordering remains valid
typedef enum {  
    A_CNCTR = 0,
    M_CNCTR,
    CTRL_FAULT,
    LEFT_BLINK,
    RIGHT_BLINK,
    Headlight_ON,
    BrakeLight,
    RSVD_LED
} light_t;

/**
* @brief   Initialize Lights Module
* @return  void
*/ 
void Lights_Init(void);

/**
* @brief   Read the state of the lights
* @param   light Which Light to read
* @return  returns state enum which indicates ON/OFF
*/ 
State Lights_Read(light_t light);

/**
 * @brief   Set light to given state
 * @param   light which light to set
 * @param   state what state to set the light to
 * @return  void
 */
void Lights_Set(light_t light, State state);

/**
 * @brief   Set multiple lights given light bitmap
 * @return  void
 */
void Lights_MultiSet(uint16_t bitmap);


/**
* @brief   Read the lights bitmap
* @return  returns uint16_t with lights bitmap
*/ 
uint16_t Lights_Bitmap_Read(light_t light);

/**
 * @brief Toggles a light. Should be used only after Toggle_Enable has been called for this light so that we are accurately tracking the enabled and disabled lights
 * @param light Which light to toggle
*/
void Lights_Toggle(light_t light);


/**
 * @brief Toggles multiple lights according to the toggle bitmap
*/
void Lights_MultiToggle(void);


/**

 * @brief   Set light toggling
 * @param   light Which light to enable toggling for
 * @param   state State to set toggling
 * @return  void
 */
void Toggle_Set(light_t light, State state);

/**
* @brief   Read the toggle enable state of a specific light from the toggle bitmap
* @param   light Which Light to read
* @return  returns State enum which indicates ON/OFF
*/
State Toggle_Read(light_t light);


/**
 * @brief   Read toggle bitmap
 * @return  returns uint16_t bitmap for toggle
 */
uint16_t Toggle_Bitmap_Read(void);

#endif
