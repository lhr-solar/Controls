#include "Ignition.h"

#define IGN_OFF_THRESHOLD 30 // tune this value based on task running frequency
#define READ_PIN_DELAY 1000 // tune this value based off switching time of Ignition switch

#define IGN_IS_OFF(motor_state, array_state) (motor_state == OFF && array_state == OFF)
#define IGN_IS_MOTOR(motor_state, array_state) (motor_state == ON && array_state == OFF)
#define IGN_IS_ARRAY(motor_state, array_state) (motor_state == OFF && array_state == ON)

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
  * @brief   Gets the state of the ignition switch
  * @return  state of the ignition switch
  */ 
 ignition_state_t Get_Ignition_State(void){
    uint8_t array_state = BSP_GPIO_Read_Pin(ARRAY_IGNITION_PORT, ARRAY_IGNITION_PIN);
    uint8_t motor_state = BSP_GPIO_Read_Pin(MOTOR_IGNITION_PORT, MOTOR_IGNITION_PIN);    
    // both pins should never be on at the same time, likely due to a short
    if(array_state == ON && motor_state == ON){
        return IGN_ERROR;
    }
    // if the ignition state has been reset previously, return the value directly
    if(ign_has_been_reset){
        if(IGN_IS_MOTOR(motor_state, array_state)){
            return IGN_MOTOR;
        }
        else if(IGN_IS_ARRAY(motor_state, array_state)){
            return IGN_ARR;
        }
    }
    else{
        // the ignition state has not been reset and is at the array or motor position
        if(!IGN_IS_OFF(motor_state, array_state)){
            return IGN_OFF;
        }
    }

    // Need to confirm if Ignition is actually off or in a floating state
    uint8_t off_counter = 0;
    while(off_counter < IGN_OFF_THRESHOLD){
        array_state = BSP_GPIO_Read_Pin(ARRAY_IGNITION_PORT, ARRAY_IGNITION_PIN);
        motor_state = BSP_GPIO_Read_Pin(MOTOR_IGNITION_PORT, MOTOR_IGNITION_PIN);
        if(IGN_IS_OFF(motor_state, array_state)){
            off_counter++;
        }
        else{
            // likely moving between states, so not actually at the off state
            if(IGN_IS_MOTOR(motor_state, array_state) || IGN_IS_ARRAY(motor_state, array_state)){
                return IGN_OFF;
            }
        }

        // dummy delay between reads
        //for(volatile int32_t i = 0; i < READ_PIN_DELAY; i++){}
    }
    // if you've escaped the while loop, igntion is off
    ign_has_been_reset = true;
    return IGN_OFF;
}