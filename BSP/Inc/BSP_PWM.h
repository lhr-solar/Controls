#ifndef __PWM_H__
#define __PWM_H__

#include "stm32f4xx.h"  // Needed for TIM_TypeDef, RCC, GPIO, etc.

#ifdef __cplusplus
extern "C" {
#endif

// Constants
#define SYSCLK_FREQ_HZ  80000000UL  // 80 MHz system clock
#define PWM_FREQ_HZ     100       // Default PWM frequency (25 kHz)
#define DUTY_CYCLE_PC   50          // Default duty cycle (50%)

// GPIO and timer init
void pwm_gpio_init(void);

// TIM1 PWM Control (CH1 -> PA8)
void pwm_tim1_init(uint32_t pwm_freq_hz, uint8_t duty_cycle_percent);
void pwm_tim1_start(void);
void pwm_tim1_stop(void);
void pwm_tim1_set_duty(uint8_t duty_percent);

// TIM8 PWM Control (CH4 -> PC9)
void pwm_tim8_init(uint32_t pwm_freq_hz, uint8_t duty_cycle_percent);
void pwm_tim8_start(void);
void pwm_tim8_stop(void);
void pwm_tim8_set_duty(uint8_t duty_percent);

#ifdef __cplusplus
}
#endif

#endif // __PWM_H__
