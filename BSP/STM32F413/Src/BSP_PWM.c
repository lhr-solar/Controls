/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "BSP_PWM.h"
#include "stm32f4xx.h"
#include "daybreak_pins.h"
#include "bsp.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"
#include "misc.h"



// Adjust these to match your clock tree
#define SYSCLK_HZ       80000000UL
#define APB1_PRESCALE   2U     // PCLK1 = SYSCLK/2?
#define APB2_PRESCALE   1U     // PCLK2 = SYSCLK/1?

// Compute optimal PSC+ARR combo for target freq.
// Finds the smallest PSC (best resolution) such that ARR ≤0xFFFF,
// and that gives the closest match to pwm_hz.
void compute_psc_arr(uint8_t on_APB2,
                            uint32_t pwm_hz,
                            uint16_t *psc,
                            uint16_t *arr)
{
    uint32_t pclk = SYSCLK_HZ /
        (on_APB2 ? APB2_PRESCALE : APB1_PRESCALE);
    // timer clock is doubled if prescaler ≥2
    if ((on_APB2 && APB2_PRESCALE > 1) ||
        (!on_APB2 && APB1_PRESCALE > 1))
    {
        pclk *= 2;
    }

    uint32_t best_err = pwm_hz;
    uint32_t best_p = 0, best_a = 0;

    for (uint32_t p = 0; p <= 0xFFFF; ++p) {
        // ideal ARR+1 = pclk / ( (p+1)*pwm_hz )
        uint32_t ideal = pclk / ((p + 1) * pwm_hz);
        if (ideal == 0) break;          // no counts left ⇒ too big PSC
        uint32_t a = ideal - 1;
        if (a > 0xFFFF) continue;       // overflow ⇒ try bigger PSC

        // check error
        uint32_t actual = pclk / ((p + 1) * (a + 1));
        uint32_t err = (actual > pwm_hz)
                     ? actual - pwm_hz
                     : pwm_hz   - actual;
        if (err < best_err) {
            best_err = err;
            best_p = p;
            best_a = a;
            if (err == 0) break;
        }
    }

    *psc = (uint16_t)best_p;
    *arr = (uint16_t)best_a;
}

// CCR = duty% of (ARR+1), rounded, clamped
uint16_t compute_ccr(uint16_t arr, uint8_t pct)
{
    if (pct > 100) pct = 100;
    uint32_t raw = ((uint32_t)arr + 1) * pct;
    // add half-divisor for rounding
    uint32_t c = (raw + 50) / 100;
    return (c > arr ? arr : (uint16_t)c);
}

// Generic PWM init for TIM1/8 style advanced timers
void pwm_timx_init(TIM_TypeDef *TIMx,
                          uint8_t on_APB2,
                          uint8_t channel,     // 1..4
                          uint32_t pwm_hz,
                          uint8_t duty_pct)
{
    // 1) Enable clock
    if (TIMx == TIM1)       RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
    else if (TIMx == TIM8)  RCC->APB2ENR |= RCC_APB2ENR_TIM8EN;

    // 2) Compute PSC & ARR
    uint16_t psc, arr;
    compute_psc_arr(on_APB2, pwm_hz, &psc, &arr);

    // 3) Write to registers
    TIMx->PSC = psc;
    TIMx->ARR = arr;

    // 4) Configure all channels in PWM1 if needed
    //    Here we do only one channel; you can duplicate for 2/3/4
    volatile uint16_t *CCMR;
    uint16_t shift_bits;
    if (channel <= 2) {
        CCMR       = &TIMx->CCMR1;
        shift_bits = (channel - 1) * 8;
    } else {
        CCMR       = &TIMx->CCMR2;
        shift_bits = (channel - 3) * 8;    // note: for CCMR2 you must subtract 3, not 1
    }

    // clear OCxM[2:0] and OCxPE
    *CCMR &= ~((0x7 << (shift_bits + 4)) | (1 << (shift_bits + 3)));

    // set PWM mode 1 (110) and preload enable
    *CCMR |=  ((0x6 << (shift_bits + 4)) | (1 << (shift_bits + 3)));

    // CCER: enable channel, active-high polarity
    TIMx->CCER |= (1 << ((channel - 1) * 4));

    // 5) ARPE + MOE
    TIMx->CR1  |= TIM_CR1_ARPE;
    TIMx->BDTR |= TIM_BDTR_MOE;

    // 6) Force update so PSC/ARR/CCR take effect
    TIMx->EGR |= TIM_EGR_UG;

    // 7) Set initial duty
    switch (channel) {
      case 1: TIMx->CCR1 = compute_ccr(arr, duty_pct); break;
      case 2: TIMx->CCR2 = compute_ccr(arr, duty_pct); break;
      case 3: TIMx->CCR3 = compute_ccr(arr, duty_pct); break;
      case 4: TIMx->CCR4 = compute_ccr(arr, duty_pct); break;
    }
}

// Wrapper for TIM1_CH1 on PA8 (AF1) and TIM8_CH4 on PC9 (AF3)
void pwm_setup(void)
{
    // GPIOA PA8 AF1 push-pull, high speed, no pull
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    GPIOA->MODER  = (GPIOA->MODER & ~(3 << 16)) | (2 << 16);
    GPIOA->OTYPER &= ~(1 <<  8);
    GPIOA->OSPEEDR |=(3 << 16);
    GPIOA->PUPDR  &= ~(3 << 16);
    GPIOA->AFR[1] = (GPIOA->AFR[1] & ~(0xF << 0)) | (1 << 0);

    // GPIOC PC9 AF3 push-pull, high speed, no pull
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
    GPIOC->MODER  = (GPIOC->MODER & ~(3 << 18)) | (2 << 18);
    GPIOC->OTYPER &= ~(1 <<  9);
    GPIOC->OSPEEDR |=(3 << 18);
    GPIOC->PUPDR  &= ~(3 << 18);
    GPIOC->AFR[1] = (GPIOC->AFR[1] & ~(0xF << 4)) | (3 << 4);

    // TIM1_CH1, 20 kHz, 50% duty
    pwm_timx_init(TIM1, /*on_APB2=*/1, 1, 4, 1);
    TIM1->CR1 |= TIM_CR1_CEN;

    // TIM8_CH4, 20 kHz, 75% duty
    pwm_timx_init(TIM8, /*on_APB2=*/1, 4, 4, 1);
    TIM8->CR1 |= TIM_CR1_CEN;
}

//------------------------------------------------------------------------------
// Start the timer (enable counting)
void pwm_timx_start(TIM_TypeDef *TIMx)
{
    TIMx->CR1 |= TIM_CR1_CEN;
}

//------------------------------------------------------------------------------
// Stop the timer (disable counting)
void pwm_timx_stop(TIM_TypeDef *TIMx)
{
    TIMx->CR1 &= ~TIM_CR1_CEN;
}

//------------------------------------------------------------------------------
// Change duty in-flight on channel 1–4.
// Will clamp 0–100% and automatically round.
//------------------------------------------------------------------------------
void pwm_timx_set_duty(TIM_TypeDef *TIMx, uint8_t channel, uint8_t duty_pct)
{
    uint16_t arr = TIMx->ARR;
    uint16_t ccr = compute_ccr(arr, duty_pct);

    switch(channel) {
      case 1:  TIMx->CCR1 = ccr;  break;
      case 2:  TIMx->CCR2 = ccr;  break;
      case 3:  TIMx->CCR3 = ccr;  break;
      case 4:  TIMx->CCR4 = ccr;  break;
      default: /* invalid channel—do nothing */ break;
    }
}