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
typedef enum {  
    A_CNCTR = 0,
    M_CNCTR,
    CTRL_FAULT,
    LEFT_BLINK,
    RIGHT_BLINK,
    Headlight_ON,
    BPS_FAULT,
    BPS_PWR,
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



#endif
