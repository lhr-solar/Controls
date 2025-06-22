/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file Lights.c
 * @brief 
 * 
 */


#include "BSP_GPIO.h"

#include "Lights.h"
// #include "daybreak_pins.h"


/**
 * @brief Initializes Lights pins
 * 
 */
void Lights_Init(void){
    BSP_GPIO_Init(RIGHT_BLINK_PORT, RIGHT_BLINK, OUTPUT, false);
    BSP_GPIO_Init(LEFT_BLINK_PORT, RIGHT_BLINK, OUTPUT, false);
    BSP_GPIO_Init(RIGHT_IND_PORT, RIGHT_IND, INPUT, false);
    BSP_GPIO_Init(LEFT_IND_PORT, RIGHT_IND, INPUT, false);    
} 

/**
 * @brief   Writes data to a specified pin
 * @param   led The led to write to
 * @param   state true=ON or false=OFF
 * @return  None
 */ 
void Lights_Write(lights_t led, bool state){
    switch (led){
        case RIGHT_LIGHT:
            BSP_GPIO_Write_Pin(RIGHT_BLINK_PORT, RIGHT_BLINK, state);
            break;
        case LEFT_LIGHT:
            BSP_GPIO_Write_Pin(LEFT_BLINK_PORT, LEFT_BLINK, state);
            break;
        default:
            break;
    }
}

/**
 * @brief   Toggles a Lights
 * @param   led The led to toggle
 * @return  None
 */ 
void Lights_Toggle(lights_t led){
    switch (led){
        case RIGHT_LIGHT:
            BSP_GPIO_Toggle_Pin(RIGHT_BLINK_PORT, RIGHT_BLINK);
            break;
        case LEFT_LIGHT:
            BSP_GPIO_Toggle_Pin(LEFT_BLINK_PORT, LEFT_BLINK);
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
        Lights_Write(i, ON);
    }
}

/**
 * @brief   Turns all Lights on
 * @return  None
 */ 
void Lights_All_Off(void){
    for(int i = 0; i < NUM_LIGHTS; i++){
        Lights_Write(i, OFF);
    }
}