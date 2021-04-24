/* Copyright (c) 2020 UT Longhorn Racing Solar */

/**
 * Source file to mimic GPIO port communication
 */ 

#include "Contactors.h"

/**
 * @brief   Initializes contactors to be used
 *          in connection with the Motor and Array
 * @param   contactor the contactor to initialize
 * @return  None
 */ 
void Contactors_Init(contactor_t contactor) {
    uint16_t pinMask = (1 << MOTOR_CNCTR_PIN)
                     | (1 << ARRAY_CNCTR_PIN);
    BSP_GPIO_Init(CONTACTORS_PORT, pinMask, true);
}

/**
 * @brief   Reads contactor states from a file 
 *          and returns the current state of 
 *          a specified contactor
 * @param   contactor specifies the contactor for 
 *          which the user would like to know its state (MOTOR/ARRAY)
 * @return  The contactor's state (ON/OFF)
 */ 
State Contactors_Get(contactor_t contactor) {
    uint16_t data = BSP_GPIO_Get_State(CONTACTORS_PORT, 
                    contactor == MOTOR ? MOTOR_CNCTR_PIN : ARRAY_CNCTR_PIN);
    return data;
}

/**
 * @brief   Sets the state of a specified contactor
 *          by updating csv file
 * @param   contactor specifies the contactor for 
 *          which the user would like to set its state (MOTOR/ARRAY)
 * @param   state specifies the state that 
 *          the user would like to set (ON/OFF)
 * @return  The contactor's state (ON/OFF)
 */ 
void Contactors_Set(contactor_t contactor, State state) {
    uint8_t pin = contactor == MOTOR ? MOTOR_CNCTR_PIN : ARRAY_CNCTR_PIN;
    BSP_GPIO_Write_Pin(CONTACTORS_PORT, pin, state);
}
