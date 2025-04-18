/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "BSP_ADC.h"
#include "stm32f4xx.h"
#include "daybreak_pins.h"

// TODO: Go back & fix this eventually to refer to some actual variable that defines this elsewhere
#define SYS_CLK_FREQ 80000000 // STM clock runs at 80MHz
#define PRESCALED_CLK_FREQ 10000000 // 10MHz

// NOTE: The following assumes that both brakelight & turn indicators will use same clock; if this isn't the case
// additional configuration (either using different channels for a given TIM or using multiple TIMs) must be done 
// NOTE: APB1 is a bus driven downstream from CPU/DMA clock master, which is used to drive downstream peripherals
#define BRAKE_LIGHT_TIM_PERIPH RCC_APB1Periph_TIM5 // Used for APB1 Periph clock enable bit (in APB1 register)
#define BRAKE_LIGHT_TIM TIM5 // Used to manage the actual hardware timer instance
#define BRAKE_LIGHT_TIM_NVIC_CHAN TIM5_IRQn // Based on TIM used, specifies which channel for the NVIC to use (which prompts interrupts)

void BSP_PWM_Init(uint32_t freq, uint32_t duty_cycle) {
	RCC_APB1PeriphClockCmd(BRAKE_LIGHT_TIM_PERIPH, ENABLE);

	BSP_GPIO_Init(TIMER_CLK_PORT, TIMER_CLK, OUTPUT, false);
	BSP_GPIO_Init(BRAKE_LIGHT_PORT, BRAKE_LIGHT, OUTPUT, false);

	// NOTE: If changing TIM_Prescalar or TIM_Period, check against GPIO_InitStruct.GPIO_Speed in BSP_GPIO.c to ensure compatibility 
    TIM_TimeBaseInitTypeDef  TB;
    TIM_TimeBaseStructInit(&TB);
    TB.TIM_Prescaler     = (SYS_CLK_FREQ / PRESCALED_CLK_FREQ) - 1; // 10MHz (7 prescale to clock divide down by 8x)
    TB.TIM_CounterMode   = TIM_CounterMode_Up; 
    TB.TIM_Period        = (PRESCALED_CLK_FREQ / freq) - 1; // 400 ticks per period (399 loaded into TIM_Period) -> more granular control
    TB.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(BRAKE_LIGHT_TIM, &TB);
    TIM_ARRPreloadConfig(BRAKE_LIGHT_TIM, ENABLE); // Enable -> when new ARR is written, takes effect next cycle 

	// Clears pending interrupt indications & arms interrupt enable register so that NVIC interrupts occur
	TIM_ClearITPendingBit(BRAKE_LIGHT_TIM, TIM_IT_Update);
	TIM_ITConfig(BRAKE_LIGHT_TIM, TIM_IT_Update, ENABLE);

	// Lower priority than CAN, SPI, & UART, which also use NVIC (Preempetion/SubPriority 0/0, 0/1, & 1/0, respectively)
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel = BRAKE_LIGHT_TIM_NVIC_CHAN;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannel = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

	// TODO: Set up initial brakelight state
	BSP_GPIO_Write(BRAKE_LIGHT_PORT)
    TIM_Cmd(BRAKE_LIGHT_TIM, ENABLE);
}

// TODO: Based off of static var determine if on/off
// TODO: set up num_ticks on vs num_ticks off, rn we have 400 total, need to figure out how to set stuff up here & in init func
void TIM2_IRQHandler(void) {

}

// TODO: Figure out how to make brakelight(s)/turn indicators play nicely when alternating between braking (more important) & indicating turning