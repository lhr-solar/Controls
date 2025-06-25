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
    pwm_gpio_init();
    pwm_tim8_init(25000,90);
    pwm_tim1_init(25000,20);
    // pwm_tim1_stop();
    // pwm_tim8_stop();
    BSP_GPIO_Init(RIGHT_IND_PORT, RIGHT_IND, INPUT, false);
    BSP_GPIO_Init(LEFT_IND_PORT, RIGHT_IND, INPUT, false);   
    pwm_tim8_start(); 
    pwm_tim1_start();
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
                pwm_tim8_set_duty(duty);
              //  pwm_tim8_start();
            break;
        case LEFT_LIGHT:
                pwm_tim1_set_duty(duty);
                pwm_tim1_start();
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
    pwm_tim1_start();
    pwm_tim8_start();
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