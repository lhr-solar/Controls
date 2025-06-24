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

// // Distinguishing between left and right turn indicators is done in hardware (thru switches)
// typedef enum {FRONT_PWM = 0, INDICATOR_PWM, BRAKE_PWM, NUM_PINS_PWM} pwm_lights_t;

// /**
//  * @brief   Initialize the PWM module
//  * @param   freq - Frequency of PWM cycles (Hz)
//  * @param	duty_cycle - Duty cycle of PWM
//  * @return  None
//  */ 
// void BSP_PWM_Init(uint32_t freq, uint16_t duty_cycle, bool ind_stay);

// /**
//  * @brief   Sets status of PWM for a certain light
//  * @param   pwm_light - Selects PWM light whose state is to be updated
//  * @param	is_active - Active/inactive status of PWM light
//  * @return  None
//  */ 
// void BSP_PWM_Set_State(pwm_lights_t pwm_light, bool is_active);

// void BSP_PWM_Set_Duty_Cycle(uint16_t duty);
// Initializes TIM1 and TIM8 PWM on PA8 and PC6 respectively
// pwm_freq_hz: Desired frequency in Hz (e.g., 25000 for 25kHz)
// duty_cycle_percent: Initial duty cycle (0-100)
void pwm_tim1_tim8_init(uint32_t pwm_freq_hz, uint8_t duty_cycle_percent);

// Start/Stop control for TIM1 PWM (on PA8)
void pwm_tim1_start(void);
void pwm_tim1_stop(void);

// Start/Stop control for TIM8 PWM (on PC6)
void pwm_tim8_start(void);
void pwm_tim8_stop(void);

// Update duty cycle for TIM1 (0-100%)
void pwm_tim1_set_duty(uint8_t duty_percent);

// Update duty cycle for TIM8 (0-100%)
void pwm_tim8_set_duty(uint8_t duty_percent);

#endif