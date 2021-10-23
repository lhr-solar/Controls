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
    if(contactor == ARRAY){
        BSP_GPIO_Init(CONTACTORS_PORT, 2, 1); //Pins 1 and 2 in Port C are output (0b110)
    }else if(contactor == MOTOR){
        BSP_GPIO_Init(CONTACTORS_PORT, 4, 1); //Pins 1 and 2 in Port C are output (0b110)
    }
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
    uint16_t data = BSP_GPIO_Read(CONTACTORS_PORT);
    switch (contactor) {
        case MOTOR:
            data = (data >> MOTOR_CNCTR_PIN) & 0x01;
            break;
        case ARRAY:
            data = (data >> ARRAY_CNCTR_PIN) & 0x01;
            break;
        default:
            break;
    }
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
    uint16_t data = BSP_GPIO_Read(CONTACTORS_PORT);
    switch (contactor) {
        case MOTOR:
            data &= ~(0x01 << MOTOR_CNCTR_PIN);
            data |= (state << MOTOR_CNCTR_PIN);
            break;
        case ARRAY:
            data &= ~(0x01 << ARRAY_CNCTR_PIN);
            data |= (state << ARRAY_CNCTR_PIN);
            break;
        default:
            break;
    }
    BSP_GPIO_Write(CONTACTORS_PORT, data);
}