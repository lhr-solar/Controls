#include "Ignition.h"

/**
 * @brief Initializes the Ignition pins
 * 
 */
 void Ignition_Init(void){
    BSP_GPIO_Init(ARRAY_IGNITION_PORT, ARRAY_IGNITION_PIN, INPUT, false);
    BSP_GPIO_Init(MOTOR_IGNITION_PORT, MOTOR_IGNITION_PIN, INPUT, false);
    BSP_GPIO_Init(CRUISE_ENABLE_PORT, CRUISE_ENABLE, INPUT, false); //Represents ignition off
 }

/**
 * @brief   Gets the state of the ignition switch
 * @return  state of the ignition switch
 */ 
ignition_state_t Get_Ignition_State(void) {
    static bool ign_has_been_reset = false; // ensures that the ignition switch is turned off before it reads as array or motor
    // static ignition_state_t prev_state = IGN_OFF;
    uint8_t array_state = BSP_GPIO_Read_Pin(ARRAY_IGNITION_PORT, ARRAY_IGNITION_PIN);
    uint8_t motor_state = BSP_GPIO_Read_Pin(MOTOR_IGNITION_PORT, MOTOR_IGNITION_PIN);    
    uint8_t off_state = BSP_GPIO_Read_Pin(CRUISE_ENABLE_PORT, CRUISE_ENABLE);    

    // none of the pins should on at the same time, likely due to a short
    if((off_state && array_state) || (motor_state && array_state) || (motor_state && off_state)){
        return IGN_ERROR;
    }

    if(off_state){
        ign_has_been_reset = true;
        return IGN_OFF;
    }
    if(!ign_has_been_reset){
        return IGN_OFF;
    }
    if(array_state){
        return IGN_ARR;
    }
    if(motor_state){
        return IGN_MOTOR;
    }

    return IGN_TRANSITION;

}