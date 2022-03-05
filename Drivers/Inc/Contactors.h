/* Copyright (c) 2020 UT Longhorn Racing Solar */

#ifndef __CONTACTORS_H
#define __CONTACTORS_H

#include "common.h"
#include "config.h"
#include "BSP_GPIO.h"
#include "stm32f4xx_gpio.h"

#define CONTACTORS_PORT PORTC
#define MOTOR_CNCTR_PIN GPIO_Pin_12
#define ARRAY_CNCTR_PIN GPIO_Pin_11 

typedef enum {ARRAY=0, MOTOR, NUM_CONTACTORS} contactor_t;

/**
 * @brief   Initializes contactors to be used
 *          in connection with the Motor and Array
 * @param   contactor the contactor to initialize (unused)
 * @return  None
 */  
void Contactors_Init(contactor_t contactor);

/**
 * @brief   Returns the current state of 
 *          a specified contactor
 * @param   contactor the contactor (MOTOR/ARRAY)
 * @return  The contactor's state (ON/OFF)
 */ 
State Contactors_Get(contactor_t contactor);

/**
 * @brief   Sets the state of a specified contactor
 * @param   contactor the contactor (MOTOR/ARRAY)
 * @param   state the state to set (ON/OFF)
 * @return  None
 */
void Contactors_Set(contactor_t contactor, State state);

#endif