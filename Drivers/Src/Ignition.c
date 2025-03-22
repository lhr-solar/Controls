#include "Ignition.h"

/**
 * @brief Initializes the Ignition pins
 * 
 */

 static bool ign_has_been_reset;
 void Ignition_Init(void){
    ign_has_been_reset = false;
    BSP_GPIO_Init(ARRAY_IGNITION_PORT, ARRAY_IGNITION_PIN, INPUT, false);
    BSP_GPIO_Init(MOTOR_IGNITION_PORT, MOTOR_IGNITION_PIN, INPUT, false);
 }

 /**
  * @brief   Toggles a status led
  * @return  state of the ignition switch
  */ 
 ignition_state_t Get_Ignition_State(void){
    // will return OFF until the ignition switch has been reset to OFF
    uint8_t array_state = BSP_GPIO_Read_Pin(ARRAY_IGNITION_PORT, ARRAY_IGNITION_PIN);
    uint8_t motor_state = BSP_GPIO_Read_Pin(MOTOR_IGNITION_PORT, MOTOR_IGNITION_PIN);
    
    // both pins should never be on at the same time, likely due to a short
    if(array_state == ON && motor_state == ON){
        return IGN_ERROR;
    }
    if(array_state == OFF && motor_state == OFF){
        ign_has_been_reset = true;
    }
    if(!ign_has_been_reset){
        return IGN_OFF;
    }
    // motor comes after array in ignition sequence, so motor implies array is on
    if(motor_state == ON){
        return IGN_MOTOR;
    }
    if(array_state == ON){
        return IGN_ARR;
    }
    return IGN_OFF;
}