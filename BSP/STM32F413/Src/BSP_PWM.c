/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "BSP_PWM.h"
#include "stm32f4xx.h"
#include "daybreak_pins.h"
#include "bsp.h"
#include "stm32f4xx_tim.h"
#include "misc.h"
#include "stm32f4xx_rcc.h"


// TODO: Go back & fix this eventually to refer to some actual variable that defines this elsewhere
#define SYS_CLK_FREQ 80000000 // STM clock runs at 80MHz
#define PRESCALED_CLK_FREQ 10000000 // 10MHz
#define FLASH_TOGGLE_COUNT 16666 // Lights must flash (full cycle) 60-120 times/min. We'll use 90 flashes/min
						 // (60 seconds/min / 90 flashes) * 25kHz 

static bool isPWMHigh = false; // Used internally for PWM bitbanging
static uint32_t indicatorCounter = 0; // Used for flashing 
static uint32_t freq = 0;
static uint8_t duty_cycle = 0;
static bool pinsActive[NUM_PINS_PWM] = {0};

void BSP_PWM_Init(uint32_t freq_arg, uint8_t duty_cycle_arg) {
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);

	BSP_GPIO_Init(TIMER_CLK_PORT, TIMER_CLK, OUTPUT, false);
	BSP_GPIO_Init(BRAKE_LIGHT_PORT, BRAKE_LIGHT, OUTPUT, false);

	// NOTE: If changing TIM_Prescalar or TIM_Period, check against GPIO_InitStruct.GPIO_Speed in BSP_GPIO.c to ensure compatibility 
    TIM_TimeBaseInitTypeDef  TB;
    TIM_TimeBaseStructInit(&TB);
    TB.TIM_Prescaler     = (SYS_CLK_FREQ / PRESCALED_CLK_FREQ) - 1; // 10MHz (7 prescale to clock divide down by 8x)
    TB.TIM_CounterMode   = TIM_CounterMode_Up; 
    TB.TIM_Period        = (PRESCALED_CLK_FREQ / freq_arg) - 1; // 400 ticks per period (399 loaded into TIM_Period) -> more granular control
    TB.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM5, &TB);
    TIM_ARRPreloadConfig(TIM5, ENABLE); // Enable -> when new ARR is written, takes effect next cycle 

	// Clears pending interrupt indications & arms interrupt enable register so that NVIC interrupts occur
	TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
	TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);

	// Lower priority than CAN, SPI, & UART, which also use NVIC (Preempetion/SubPriority 0/0, 0/1, & 1/0, respectively)
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel = TIM5_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

	// Set up initial brakelight state
	BSP_GPIO_Write_Pin(BRAKE_LIGHT_PORT, BRAKE_LIGHT, false);
	BSP_GPIO_Write_Pin(TIMER_CLK_PORT, TIMER_CLK, false);
	// TODO: Front lights aura turn on

	// Set our freq and duty cycle
	freq = freq_arg;
	duty_cycle = duty_cycle_arg;
    TIM_Cmd(TIM5, ENABLE);
}

// such that the num_high_ticks & num_low_ticks add up to 400 & work with the desired duty cycle (not difficult, just WIP)
void TIM5_IRQHandler(void) {
	TIM_ClearITPendingBit(TIM5, TIM_IT_Update); //clear inturrupt flag

	// Reset indicator counter
	if(indicatorCounter >= FLASH_TOGGLE_COUNT) indicatorCounter = 0;
	if(indicatorCounter == FLASH_TOGGLE_COUNT / 2) BSP_GPIO_Write_Pin(TIMER_CLK_PORT, TIMER_CLK, false);

	// PWM Pin logic
	// TODO: Add logic that turns off stuff completely if pins aren't active
	if(isPWMHigh) {
		// Turn on
		if(pinsActive[BRAKE_PWM]) {
			BSP_GPIO_Write_Pin(BRAKE_LIGHT_PORT, BRAKE_LIGHT, true);
		}
		if(pinsActive[INDICATOR_PWM] && indicatorCounter < FLASH_TOGGLE_COUNT / 2) {
			BSP_GPIO_Write_Pin(TIMER_CLK_PORT, TIMER_CLK, true);
		}
		TIM5->ARR = ((PRESCALED_CLK_FREQ / freq) * duty_cycle) / 100;
	} 
	else {
		// Turn off
		if(pinsActive[BRAKE_PWM]) {
			BSP_GPIO_Write_Pin(BRAKE_LIGHT_PORT, BRAKE_LIGHT, false);
		}
		if(pinsActive[INDICATOR_PWM] && indicatorCounter < FLASH_TOGGLE_COUNT / 2) {
			BSP_GPIO_Write_Pin(TIMER_CLK_PORT, TIMER_CLK, false);
		}
		TIM5->ARR = ((PRESCALED_CLK_FREQ / freq) * (100 - duty_cycle)) / 100;
	}
	indicatorCounter++;
	isPWMHigh = !isPWMHigh;
}

void BSP_PWM_Set_State(pwm_lights_t pwm_light, bool is_active) {
	pinsActive[pwm_light] = is_active;
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