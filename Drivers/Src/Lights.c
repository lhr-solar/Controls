/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file Lights.c
 * @brief 
 * 
 */


#include "BSP_GPIO.h"

#include "stm32f4xx.h"

#include "BSP_PWM.h"

#include "Lights.h"


/**
 * @brief Initializes Lights pins
 * 
 */
void Lights_Init(void){
    pwm_setup(); // Init TIM8 and TIM1 for PA8 and PC9
    BSP_GPIO_Init(RIGHT_IND_PORT, RIGHT_IND, INPUT, false);
    BSP_GPIO_Init(LEFT_IND_PORT, LEFT_IND, INPUT, false);   
    BSP_GPIO_Init(BRAKE_LIGHT_PORT, BRAKE_LIGHT, OUTPUT, false);

} 

/**
 * @brief   Writes data to a specified pin
 * @param   led The led to write to
 * @param   state true=ON or false=OFF
 * @return  None
 */ 
void Lights_Write(lights_t led, bool state){ // tim1 = port a = left 
    switch (led){
        case RIGHT_LIGHT:
                pwm_timx_set_duty(TIM8,4,(state ? 50 : 0));
                pwm_timx_start(TIM8);
            break;
        case LEFT_LIGHT:
                pwm_timx_set_duty(TIM1,1,(state ? 50 : 0));
                pwm_timx_start(TIM1);
            break;
        case BRAKE_LIGHT:
                BSP_GPIO_Write_Pin(BRAKE_LIGHT_PORT, BRAKE_LIGHT_PIN, ON);    
        default:
            break;
    }
}

/**
 * @brief   Turns all Lights on
 * @return  None
 */ 
void Lights_All_On(void){
    for(int i = 0; i < NUM_LIGHTS; i++){
        Lights_Write(i, 100);
    }
}

/**
 * @brief   Turns all Lights on
 * @return  None
 */ 
void Lights_All_Off(void){
    for(int i = 0; i < NUM_LIGHTS; i++){
        Lights_Write(i, 0);
    }
}