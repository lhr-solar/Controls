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

static uint32_t indicatorCounter = 0; // Used for flashing
static uint32_t pwmCounter = 0; // Used for PWM duty cycle control
static uint32_t freq = 0;
static uint16_t duty_cycle = 0;
static bool pinsActive[NUM_PINS_PWM] = {0};
static bool indicator_stay_on = 0;

void BSP_PWM_Init(uint32_t freq_arg, uint16_t duty_cycle_arg, bool ind_stay) {
    indicator_stay_on = ind_stay;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);

    BSP_GPIO_Init(TIMER_CLK_PORT, TIMER_CLK, OUTPUT, false);
    BSP_GPIO_Init(BRAKE_LIGHT_PORT, BRAKE_LIGHT, OUTPUT, false);

    // Timer configuration - KEEP THIS FIXED, don't modify ARR in the ISR
    TIM_TimeBaseInitTypeDef TB;
    TIM_TimeBaseStructInit(&TB);
    TB.TIM_Prescaler = (SYS_CLK_FREQ / PRESCALED_CLK_FREQ) - 1; // 10MHz
    TB.TIM_CounterMode = TIM_CounterMode_Up; 
    TB.TIM_Period = (PRESCALED_CLK_FREQ / freq_arg) - 1; // Fixed period
    TB.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM5, &TB);
    TIM_ARRPreloadConfig(TIM5, ENABLE);

    // Interrupt configuration
    TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
    TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);

    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = TIM5_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    // Initial state
    BSP_GPIO_Write_Pin(BRAKE_LIGHT_PORT, BRAKE_LIGHT, false);
    BSP_GPIO_Write_Pin(TIMER_CLK_PORT, TIMER_CLK, false);

    // Set parameters
    freq = freq_arg;
    duty_cycle = duty_cycle_arg;
    
    // Start timer
    TIM_Cmd(TIM5, ENABLE);
}

void TIM5_IRQHandler(void) {
    TIM_ClearITPendingBit(TIM5, TIM_IT_Update);

    // Update PWM counter for duty cycle control
    pwmCounter = (pwmCounter + 1) % PWM_RESOLUTION;
    
    // Update indicator flash counter
    indicatorCounter++;
    if(indicatorCounter >= FLASH_TOGGLE_COUNT) {
        indicatorCounter = 0;
    }
    
    // Determine if we're in the ON phase of the indicator flashing
    bool indicator_on_phase = (indicatorCounter < FLASH_TOGGLE_COUNT / 2);
    
    // Apply PWM to brake light
    if(pinsActive[BRAKE_PWM]) {
        // Pin HIGH if counter is less than duty cycle
        bool brake_state = (pwmCounter < duty_cycle);
        BSP_GPIO_Write_Pin(BRAKE_LIGHT_PORT, BRAKE_LIGHT, brake_state);
    }
    
    // Apply PWM to indicator light (only during ON phase)
    if(pinsActive[INDICATOR_PWM]) {
        if(indicator_on_phase || indicator_stay_on) {
            // Apply PWM during ON phase of flashing
            bool indicator_state = (pwmCounter < duty_cycle);
            BSP_GPIO_Write_Pin(TIMER_CLK_PORT, TIMER_CLK, indicator_state);
        } else {
            // Force OFF during OFF phase of flashing
            BSP_GPIO_Write_Pin(TIMER_CLK_PORT, TIMER_CLK, false);
        }
    }
}

void BSP_PWM_Set_State(pwm_lights_t pwm_light, bool is_active) {
    pinsActive[pwm_light] = is_active;
    
    // Immediately turn off when deactivating
    if(!is_active) {
        switch(pwm_light) {
            case FRONT_PWM:
                // TODO: 
                break;
            case INDICATOR_PWM:
                BSP_GPIO_Write_Pin(TIMER_CLK_PORT, TIMER_CLK, false);
                break;
            case BRAKE_PWM:
                BSP_GPIO_Write_Pin(BRAKE_LIGHT_PORT, BRAKE_LIGHT, false);
                break;
            default:
                break;
        }
    }
}

void BSP_PWM_Set_Duty_Cycle(uint16_t duty){
	duty_cycle = duty;
}
