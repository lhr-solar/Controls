/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file Lights.c
 * @brief 
 * 
 */


#include "BSP_GPIO.h"

#include "BSP_PWM.h"

#include "Lights.h"
// #include "daybreak_pins.h"


/**
 * @brief Initializes Lights pins
 * 
 */
void Lights_Init(void){
    pwm_tim1_tim8_init(25000,20);
    pwm_tim1_stop();
    pwm_tim8_stop();
    BSP_GPIO_Init(RIGHT_IND_PORT, RIGHT_IND, INPUT, false);
    BSP_GPIO_Init(LEFT_IND_PORT, RIGHT_IND, INPUT, false);    
} 

/**
 * @brief   Writes data to a specified pin
 * @param   led The led to write to
 * @param   state true=ON or false=OFF
 * @return  None
 */ 
void Lights_Write(lights_t led, uint8_t duty){ // tim1 = port a = left 
    switch (led){
        case RIGHT_LIGHT:
            if (duty == 0){
                pwm_tim8_stop();
            }
            else {
                pwm_tim8_set_duty(duty);
            }
            break;
        case LEFT_LIGHT:
            if (duty == 0){
                pwm_tim1_stop();
            }
            else {
                pwm_tim1_set_duty(duty);
            }
            break;
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