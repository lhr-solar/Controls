/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "BSP_PWM.h"
#include "stm32f4xx.h"
#include "daybreak_pins.h"
#include "bsp.h"
#include "stm32f4xx_tim.h"
#include "misc.h"
#include "stm32f4xx_rcc.h"

// Clock and timing definitions
#define SYS_CLK_FREQ 80000000 // STM clock runs at 80MHz
#define PRESCALED_CLK_FREQ 10000000 // 10MHz
#define FLASH_TOGGLE_COUNT 16666 // For 90 flashes/min at 25kHz
#define PWM_RESOLUTION 100 // Resolution for duty cycle (0-100)

#define SYSCLK_FREQ_HZ  80000000UL  // 80 MHz system clock
#define PWM_FREQ_HZ     100      // 25 kHz PWM
#define DUTY_CYCLE_PC   50          // Default 50% duty

static uint32_t compute_arr(uint32_t pwm_freq_hz) {
    return (SYSCLK_FREQ_HZ / pwm_freq_hz) - 1;
}

static uint32_t compute_ccr(uint32_t arr, uint8_t duty_percent) {
    if (duty_percent > 100) duty_percent = 100;
    return ((arr + 1) * duty_percent) / 100;
}

void pwm_gpio_init(void) {
    // Enable GPIOA and GPIOC clocks
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOCEN;

    // PA8 -> TIM1_CH1 (AF1)
    GPIOA->MODER &= ~(3 << (8 * 2));
    GPIOA->MODER |=  (2 << (8 * 2));
    GPIOA->AFR[1] &= ~(0xF << ((8 - 8) * 4));
    GPIOA->AFR[1] |=  (1 << ((8 - 8) * 4));  // AF1 for TIM1_CH1

    // PC9 -> TIM8_CH4 (AF3)
    GPIOC->MODER &= ~(3 << (9 * 2));
    GPIOC->MODER |=  (2 << (9 * 2));
    GPIOC->AFR[1] &= ~(0xF << ((9 - 8) * 4));
    GPIOC->AFR[1] |=  (3 << ((9 - 8) * 4));  // AF3 for TIM8_CH4
}

void pwm_tim1_init(uint32_t pwm_freq_hz, uint8_t duty_cycle_percent) {
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;

    uint32_t arr = compute_arr(pwm_freq_hz);
    uint32_t ccr = compute_ccr(arr, duty_cycle_percent);

    TIM1->PSC = 0;
    TIM1->ARR = arr;
    TIM1->CCR1 = ccr;

    TIM1->CCMR1 &= ~TIM_CCMR1_OC1M;
    TIM1->CCMR1 &= ~TIM_CCMR1_OC1M; // Clear previous mode bits
    TIM1->CCMR1 |= TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2; // PWM mode 1 (0b110)
    TIM1->CCMR1 |= TIM_CCMR1_OC1PE;

    TIM1->CCER |= TIM_CCER_CC1E;

    TIM1->CR1 |= TIM_CR1_ARPE;
    TIM1->BDTR |= TIM_BDTR_MOE;
}

void pwm_tim8_init(uint32_t pwm_freq_hz, uint8_t duty_cycle_percent) {
    RCC->APB2ENR |= RCC_APB2ENR_TIM8EN;

    uint32_t arr = compute_arr(pwm_freq_hz);
    uint32_t ccr = compute_ccr(arr, duty_cycle_percent);

    TIM8->PSC = 0;
    TIM8->ARR = arr;
    TIM8->CCR4 = ccr;

    TIM8->CCMR2 &= ~TIM_CCMR2_OC4M;
    TIM8->CCMR2 &= ~TIM_CCMR2_OC4M;
    TIM8->CCMR2 |= TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4M_2;
    TIM8->CCMR2 |= TIM_CCMR2_OC4PE;

    TIM8->CCER |= TIM_CCER_CC4E;

    TIM8->CR1 |= TIM_CR1_ARPE;
    TIM8->BDTR |= TIM_BDTR_MOE;
}

void pwm_tim1_start(void) {
    TIM1->CR1 |= TIM_CR1_CEN;
}

void pwm_tim8_start(void) {
    TIM8->CR1 |= TIM_CR1_CEN;
}

void pwm_tim1_stop(void) {
    TIM1->CR1 &= ~TIM_CR1_CEN;
}

void pwm_tim8_stop(void) {
    TIM8->CR1 &= ~TIM_CR1_CEN;
}

void pwm_tim1_set_duty(uint8_t duty_percent) {
    uint32_t arr = TIM1->ARR;
    TIM1->CCR1 = compute_ccr(arr, duty_percent);
}

void pwm_tim8_set_duty(uint8_t duty_percent) {
    uint32_t arr = TIM8->ARR;
    TIM8->CCR4 = compute_ccr(arr, duty_percent);
}



//}