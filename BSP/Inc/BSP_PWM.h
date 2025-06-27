#ifndef __PWM_H__
#define __PWM_H__

#include "stm32f4xx.h"  // Needed for TIM_TypeDef, RCC, GPIO, etc.

#ifdef __cplusplus
extern "C" {
#endif

// Adjust these to match your clock tree
#define SYSCLK_HZ       80000000UL
#define APB1_PRESCALE   2U     // PCLK1 = SYSCLK/2?
#define APB2_PRESCALE   1U     // PCLK2 = SYSCLK/1?

void compute_psc_arr(uint8_t on_APB2,
                            uint32_t pwm_hz,
                            uint16_t *psc,
                            uint16_t *arr);

uint16_t compute_ccr(uint16_t arr, uint8_t pct);

void pwm_timx_init(TIM_TypeDef *TIMx,
                          uint8_t on_APB2,
                          uint8_t channel,     // 1..4
                          uint32_t pwm_hz,
                          uint8_t duty_pct);

void pwm_setup(void);

void pwm_timx_start(TIM_TypeDef *TIMx);

void pwm_timx_stop(TIM_TypeDef *TIMx);

void pwm_timx_set_duty(TIM_TypeDef *TIMx, uint8_t channel, uint8_t duty_pct);

#ifdef __cplusplus
}
#endif

#endif // __PWM_H__
