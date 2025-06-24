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

// static uint32_t indicatorCounter = 0; // Used for flashing
// static uint32_t pwmCounter = 0; // Used for PWM duty cycle control
// static uint32_t freq = 0;
// static uint16_t duty_cycle = 0;
// static bool pinsActive[NUM_PINS_PWM] = {0};
// static bool indicator_stay_on = 0;

// void BSP_PWM_Init(uint32_t freq_arg, uint16_t duty_cycle_arg, bool ind_stay) {
//     indicator_stay_on = ind_stay;
	
// 	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);

//     BSP_GPIO_Init(TIMER_CLK_PORT, TIMER_CLK, OUTPUT, false);
//     BSP_GPIO_Init(BRAKE_LIGHT_PORT, BRAKE_LIGHT, OUTPUT, false);

//     // Timer configuration - KEEP THIS FIXED, don't modify ARR in the ISR
//     TIM_TimeBaseInitTypeDef TB;
//     TIM_TimeBaseStructInit(&TB);
//     TB.TIM_Prescaler = (SYS_CLK_FREQ / PRESCALED_CLK_FREQ) - 1; // 10MHz
//     TB.TIM_CounterMode = TIM_CounterMode_Up; 
//     TB.TIM_Period = (PRESCALED_CLK_FREQ / freq_arg) - 1; // Fixed period
//     TB.TIM_ClockDivision = TIM_CKD_DIV1;
//     TIM_TimeBaseInit(TIM5, &TB);
//     TIM_ARRPreloadConfig(TIM5, ENABLE);

//     // Interrupt configuration
//     TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
//     TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);

//     NVIC_InitTypeDef NVIC_InitStruct;
//     NVIC_InitStruct.NVIC_IRQChannel = TIM5_IRQn;
//     NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
//     NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
//     NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
//     NVIC_Init(&NVIC_InitStruct);

//     // Initial state
//     BSP_GPIO_Write_Pin(BRAKE_LIGHT_PORT, BRAKE_LIGHT, false);
//     BSP_GPIO_Write_Pin(TIMER_CLK_PORT, TIMER_CLK, false);

//     // Set parameters
//     freq = freq_arg;
//     duty_cycle = duty_cycle_arg;
    
//     // Start timer
//     TIM_Cmd(TIM5, ENABLE);
// }

// void TIM5_IRQHandler(void) {
//     TIM_ClearITPendingBit(TIM5, TIM_IT_Update);

//     // Update PWM counter for duty cycle control
//     pwmCounter = (pwmCounter + 1) % PWM_RESOLUTION;
    
//     // Update indicator flash counter
//     indicatorCounter++;
//     if(indicatorCounter >= FLASH_TOGGLE_COUNT) {
//         indicatorCounter = 0;
//     }
    
//     // Determine if we're in the ON phase of the indicator flashing
//     bool indicator_on_phase = (indicatorCounter < FLASH_TOGGLE_COUNT / 2);
    
//     // Apply PWM to brake light
//     if(pinsActive[BRAKE_PWM]) {
//         // Pin HIGH if counter is less than duty cycle
//         bool brake_state = (pwmCounter < duty_cycle);
//         BSP_GPIO_Write_Pin(BRAKE_LIGHT_PORT, BRAKE_LIGHT, brake_state);
//     }
    
//     // Apply PWM to indicator light (only during ON phase)
//     if(pinsActive[INDICATOR_PWM]) {
//         if(indicator_on_phase || indicator_stay_on) {
//             // Apply PWM during ON phase of flashing
//             bool indicator_state = (pwmCounter < duty_cycle);
//             BSP_GPIO_Write_Pin(TIMER_CLK_PORT, TIMER_CLK, indicator_state);
//         } else {
//             // Force OFF during OFF phase of flashing
//             BSP_GPIO_Write_Pin(TIMER_CLK_PORT, TIMER_CLK, false);
//         }
//     }
// }

// void BSP_PWM_Set_State(pwm_lights_t pwm_light, bool is_active) {
//     pinsActive[pwm_light] = is_active;
    
//     // Immediately turn off when deactivating
//     if(!is_active) {
//         switch(pwm_light) {
//             case FRONT_PWM:
//                 // TODO: 
//                 break;
//             case INDICATOR_PWM:
//                 BSP_GPIO_Write_Pin(TIMER_CLK_PORT, TIMER_CLK, false);
//                 break;
//             case BRAKE_PWM:
//                 BSP_GPIO_Write_Pin(BRAKE_LIGHT_PORT, BRAKE_LIGHT, false);
//                 break;
//             default:
//                 break;
//         }
//     }
// }

// void BSP_PWM_Set_Duty_Cycle(uint16_t duty){
// 	duty_cycle = duty;
// }


// // void BSP_PWM1_Init(uint16_t duty){


//     // frequency from TIM_ARR_ARR
//     // duty cycle from TIM_CCR1_CCR1
//     TIM_ARR_ARR
//     TIM_CCR1_CCR1
//     TIM_CCMR1_OC1M


    // set 110 for PWM mode 1 the OCxM bits in the TIMx_CCMRx register

    // preload register must be enabled in the OCxPE bit in the TIMx_CCMRx register
    // and auto-reload preload register by setting the ARPE bit in the TIMx_CR1 register

    // BEFORE STARTING all registers must be initialized by setting the UG bit in the TIMx_EGR register

    // god bless chat
#define PWM_FREQ_HZ    25000    // 25 kHz is a safe choice for visible LEDs
#define DUTY_CYCLE_PC  50       // Start at 50% brightness
#define SYSCLK_FREQ_HZ  80000000UL  // System clock frequency (e.g. 80 MHz)

void pwm_tim1_tim8_init(uint32_t pwm_freq_hz, uint8_t duty_cycle_percent) {
    // 1. Enable clocks for GPIOA and GPIOC, TIM1, TIM8
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOCEN;
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN | RCC_APB2ENR_TIM8EN;

    // 2. Set GPIOA8 (TIM1_CH1) and GPIOC6 (TIM8_CH1) to Alternate Function
    GPIOA->MODER &= ~(3 << (8 * 2));
    GPIOA->MODER |=  (2 << (8 * 2));         // AF mode for PA8
    GPIOA->AFR[1] &= ~(0xF << (0 * 4));      // Clear AFR8
    GPIOA->AFR[1] |=  (1 << (0 * 4));        // AF1 for TIM1_CH1

    GPIOC->MODER &= ~(3 << (9 * 2));
    GPIOC->MODER |=  (2 << (9 * 2));         // AF mode for PC9
    GPIOC->AFR[1] &= ~(0xF << ((9 - 8) * 4));
    GPIOC->AFR[1] |=  (3 << ((9 - 8) * 4));   // AF3 for TIM8_CH4


    // 3. Timer configuration
    uint32_t timer_clock = SYSCLK_FREQ_HZ;  // APB2 = SYSCLK if prescaler = 1
    uint32_t period = (timer_clock / pwm_freq_hz) - 1;
    uint32_t pulse = (period + 1) * duty_cycle_percent / 100;

    // --- TIM1 Configuration ---
    TIM1->PSC = 0;                   // No prescaler
    TIM1->ARR = period;             // Auto-reload value
    TIM1->CCR1 = pulse;             // Duty cycle
    TIM1->CCMR1 &= ~TIM_CCMR1_OC1M;
    TIM1->CCMR1 |= (6 << 4);  // PWM mode 1 (OC1M = 110)
    TIM1->CCMR1 |= TIM_CCMR1_OC1PE;            // Enable preload
    TIM1->CCER |= TIM_CCER_CC1E;               // Enable output on CH1
    TIM1->CR1 |= TIM_CR1_ARPE;                 // Enable auto-reload preload
    TIM1->BDTR |= TIM_BDTR_MOE;                // Main output enable (TIM1/TIM8 only)
    TIM1->CR1 |= TIM_CR1_CEN;                  // Enable counter

    // --- TIM8 Configuration ---
    TIM8->PSC = 0;
    TIM8->ARR = period;
    TIM8->CCR4 = pulse;
    TIM8->CCMR2 &= ~TIM_CCMR2_OC4M;
    TIM8->CCMR2 |= (6 << 12); // PWM mode 1
    TIM8->CCMR2 |= TIM_CCMR2_OC4PE;
    TIM8->CCER |= TIM_CCER_CC4E;
}

void pwm_tim1_start(void) {
    TIM1->BDTR |= TIM_BDTR_MOE;    // Main output enable
    TIM1->CCER |= TIM_CCER_CC1E;   // Enable output on CH1
    TIM1->CR1 |= TIM_CR1_CEN;      // Start counter
}

void pwm_tim1_stop(void) {
    TIM1->CR1 &= ~TIM_CR1_CEN;     // Stop counter
    TIM1->CCER &= ~TIM_CCER_CC1E;  // Disable output
    TIM1->BDTR &= ~TIM_BDTR_MOE;   // Disable main output
}

void pwm_tim8_start(void) {
    TIM8->BDTR |= TIM_BDTR_MOE;      // Enable main output (required for advanced timers)
    TIM8->CCER |= TIM_CCER_CC4E;     // Enable CH4 output
    TIM8->CR1 |= TIM_CR1_CEN;        // Enable counter
}

void pwm_tim8_stop(void) {
    TIM8->CR1 &= ~TIM_CR1_CEN;       // Disable counter
    TIM8->CCER &= ~TIM_CCER_CC4E;    // Disable CH4 output
    TIM8->BDTR &= ~TIM_BDTR_MOE;     // Disable main output
}


void pwm_tim1_set_duty(uint8_t duty_percent) {
    uint32_t period = TIM1->ARR + 1;
    TIM1->CCR1 = (period * duty_percent) / 100;
}

void pwm_tim8_set_duty(uint8_t duty_percent) {
    uint32_t period = TIM8->ARR + 1;
    TIM8->CCR4 = (period * duty_percent) / 100;
}





//}