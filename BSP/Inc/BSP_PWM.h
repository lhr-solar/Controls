#ifndef BSP_PWM_H
#define BSP_PWM_H

#include "stm32f4xx.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "config.h"
#include "common.h"

#define PWM_PERIOD 4000

/**
 * @brief   Initialize all the GPIO pins meant for pwm
 * @param   None
 * @return  None
 */
void BSP_PWM_Init(void);

/**
 * @brief   Sets a pin's duty cycle
 * @param   dutyCycle: duty cycle amount between 0 and the period
 * @param   pin: pin number whose speed should be changed
 * @return  ErrorStatus
 */
void BSP_PWM_Set(uint8_t pin, uint32_t speed);

/**
 * @brief   Get current duty cycle of a single pin, return -1 if input is invalid
 * @param   pin Number
 * @return  Current PWM duty cycle of pin
 */
int BSP_PWM_Get(uint8_t pin);

#endif