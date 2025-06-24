#include "BSP_PWM.h"

GPIO_InitTypeDef GPIO_INIT_STRUCT;
TIM_OCInitTypeDef TIMER_OC_STRUCT;
TIM_TimeBaseInitTypeDef TIMER_INIT_STRUCT;

/**
 * @brief   Initialize all the GPIO pins meant for pwm
 * @param   None
 * @return  None
 */
void BSP_PWM_Init(void){

    // PC9 = Right Blink = TIM3_CH4
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); // Enable Timer
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE); // Enable Port Clock
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource9, GPIO_AF_TIM3); // Set alternate function

    GPIO_INIT_STRUCT.GPIO_Mode = GPIO_Mode_AF;
    GPIO_INIT_STRUCT.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_INIT_STRUCT.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_INIT_STRUCT.GPIO_OType = GPIO_OType_PP;
    GPIO_INIT_STRUCT.GPIO_Pin = GPIO_Pin_9;
    GPIO_Init(GPIOC, &GPIO_INIT_STRUCT); // Configure PC9 for alternate function


    TIMER_INIT_STRUCT.TIM_Period = PWM_PERIOD;
  	TIMER_INIT_STRUCT.TIM_Prescaler = 0x0000;
	TIMER_INIT_STRUCT.TIM_ClockDivision = TIM_CKD_DIV1;
	TIMER_INIT_STRUCT.TIM_CounterMode = TIM_CounterMode_Up;
	TIMER_INIT_STRUCT.TIM_RepetitionCounter = 0x0000;
    TIM_TimeBaseInit(TIM3, &TIMER_INIT_STRUCT);

    // Configure TIM3 Channel 4 for PWM
    TIMER_OC_STRUCT.TIM_OCMode = TIM_OCMode_PWM1;
    TIMER_OC_STRUCT.TIM_OutputState = TIM_OutputState_Enable;
    TIMER_OC_STRUCT.TIM_OutputNState = TIM_OutputNState_Disable;
    TIMER_OC_STRUCT.TIM_Pulse = 0; // Start with everything off
    TIMER_OC_STRUCT.TIM_OCPolarity = TIM_OCPolarity_High;
    TIMER_OC_STRUCT.TIM_OCNPolarity = TIM_OCPolarity_High;
    TIMER_OC_STRUCT.TIM_OCIdleState = TIM_OCIdleState_Reset;
    TIMER_OC_STRUCT.TIM_OCNIdleState = TIM_OCNIdleState_Reset;
    TIM_OC4Init(TIM3, &TIMER_OC_STRUCT);

    // Enable the output compare preload for channel 4
    TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);

    // Enable auto-reload preload
    TIM_ARRPreloadConfig(TIM3, ENABLE);

    // Enable TIM3 counter
    TIM_Cmd(TIM3, ENABLE);
}

/**
 * @brief   Sets a pin's duty cycle
 * @param   dutyCycle: duty cycle amount between 0 and the period
 * @param   pin: pin number whose speed should be changed
 */
void BSP_PWM_Set(uint8_t pin, uint32_t speed){

}

/**
 * @brief   Get current duty cycle of a single pin, return -1 if input is invalid
 * @param   pin Number
 * @return  Current PWM duty cycle of pin
 */
int BSP_PWM_Get(uint8_t pin){
    return -1;

}