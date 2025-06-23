#include "Lights.h"

/**
 * @brief Initializes light pins
 * @return None
 */
void Lights_Init(void){

    // Initialize the right and left lights
    BSP_GPIO_Init(RIGHT_BLINK_PORT, RIGHT_BLINK, OUTPUT, false);
    BSP_GPIO_Init(LEFT_BLINK_PORT, LEFT_BLINK, OUTPUT, false);


    // Initialize the brakelight port
    BSP_GPIO_Init(BRAKE_LIGHT_PORT, BRAKE_LIGHT_PIN, OUTPUT, false);
}

/**
 * @brief   Writes data to a specified pin
 * @param   led The led to write to
 * @param   state true=ON or false=OFF
 * @return  None
 */ 
void Lights_Write(light_t led, bool state){
    switch(led){
        case RIGHT_BLINK_LIGHT:
            BSP_GPIO_Write_Pin(RIGHT_BLINK_PORT, RIGHT_BLINK, state);
            break;
        case LEFT_BLINK_LIGHT:
            BSP_GPIO_Write_Pin(LEFT_BLINK_PORT, LEFT_BLINK, state);
            break;
        case BRAKE_LIGHT:
            BSP_GPIO_Write_Pin(BRAKE_LIGHT_PORT, BRAKE_LIGHT_PIN, state);
            break;
        default:
            break;
    }

}