/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file BSP_PWM.h
 * @brief Header file for the library to interact
 * with the Timer (PWM)
 * 
 * @defgroup BSP_PWM
 * @addtogroup BSP_PWM
 * @{
 */

#ifndef __BSP_PWM_H
#define __BSP_PWM_H

#include "bsp.h"
#include "common.h"
#include "config.h"
#include "daybreak_pins.h"

// Distinguishing between left and right turn indicators is done in hardware (thru switches)
typedef enum {FRONT_PWM = 0, INDICATOR_PWM, BRAKE_PWM, NUM_PINS} pwm_lights_t;
bool pinsActive[NUM_PINS] = {0};

/**
 * @brief   Initialize the PWM module
 * @param   freq - Frequency of PWM cycles (Hz)
 * @param	duty_cycle - Duty cycle of PWM
 * @return  None
 */ 
void BSP_PWM_Init(uint32_t freq, uint32_t duty_cycle);

/**
 * @brief   Sets status of PWM for a certain light
 * @param   pwm_light - Selects PWM light whose state is to be updated
 * @param	is_active - Active/inactive status of PWM light
 * @return  None
 */ 
void BSP_PWM_Set_State(pwm_lights_t pwm_light, bool is_active);


#endif
