/* Copyright (c) 2020 UT Longhorn Racing Solar */


#include "Contactors.h"


/**
 * @brief   Initializes contactors to be used
 *          in connection with the Motor and Array
 * @param   contactor the contactor to initialize (unused)
 * @return  None
 */ 
void Contactors_Init(contactor_t contactor) {
    BSP_GPIO_Init(CONTACTORS_PORT, 
                 MOTOR_CNCTR_PIN | ARRAY_CNCTR_PIN, 
                  1);
}

/**
 * @brief   Returns the current state of 
 *          a specified contactor
 * @param   contactor the contactor (MOTOR/ARRAY)
 * @return  The contactor's state (ON/OFF)
 */ 
State Contactors_Get(contactor_t contactor) {
    State state = OFF;
    switch (contactor) {
        case MOTOR:
            state = BSP_GPIO_Read_Pin(CONTACTORS_PORT, MOTOR_CNCTR_PIN);
            break;
        case ARRAY:
            state = BSP_GPIO_Read_Pin(CONTACTORS_PORT, ARRAY_CNCTR_PIN);
            break;
        default:
            break;
    }
    return state;
}

/**
 * @brief   Sets the state of a specified contactor
 * @param   contactor the contactor (MOTOR/ARRAY)
 * @param   state the state to set (ON/OFF)
 * @return  None
 */
void Contactors_Set(contactor_t contactor, State state) {
    switch (contactor) {
        case MOTOR:
            BSP_GPIO_Write_Pin(CONTACTORS_PORT, MOTOR_CNCTR_PIN, state);
            break;
        case ARRAY:
            BSP_GPIO_Write_Pin(CONTACTORS_PORT, ARRAY_CNCTR_PIN, state);
            break;
        default:
            break;
    }
}