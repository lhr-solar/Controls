/* Copyright (c) 2020 UT Longhorn Racing Solar */


#include "Contactors.h"
#include "stm32f4xx_gpio.h"

#define CONTACTORS_PORT         PORTC
#define ARRAY_PRECHARGE_PIN GPIO_Pin_10
#define ARRAY_CONTACTOR_PIN GPIO_Pin_11
#define MOTOR_CONTACTOR_PIN GPIO_Pin_12


/**
 * @brief   Initializes contactors to be used
 *          in connection with the Motor and Array
 * @return  None
 */ 
void Contactors_Init() {
    BSP_GPIO_Init(CONTACTORS_PORT, 
                 (ARRAY_CONTACTOR_PIN) | 
                 (ARRAY_PRECHARGE_PIN) |
                 (MOTOR_CONTACTOR_PIN), 
                  1);
}

/**
 * @brief   Returns the current state of 
 *          a specified contactor
 * @param   contactor the contactor
 *              (MOTOR_PRECHARGE/ARRAY_PRECHARGE/ARRAY_CONTACTOR)
 * @return  The contactor's state (ON/OFF)
 */ 
State Contactors_Get(contactor_t contactor) {
    State state = OFF;
    switch (contactor) {
        case ARRAY_CONTACTOR :
            state = BSP_GPIO_Read_Pin(CONTACTORS_PORT, ARRAY_CONTACTOR_PIN);
            break;
        case ARRAY_PRECHARGE :
            state = BSP_GPIO_Read_Pin(CONTACTORS_PORT, ARRAY_PRECHARGE_PIN);
            break;
        case MOTOR_CONTACTOR :
            state = BSP_GPIO_Read_Pin(CONTACTORS_PORT, MOTOR_CONTACTOR_PIN);
            break;
        default:
            break;
    }
    return state;
}

/**
 * @brief   Sets the state of a specified contactor
 * @param   contactor the contactor
 *              (MOTOR_PRECHARGE/ARRAY_PRECHARGE/ARRAY_CONTACTOR)
 * @param   state the state to set (ON/OFF)
 * @return  None
 */
void Contactors_Set(contactor_t contactor, State state) {
    switch (contactor) {
        case ARRAY_CONTACTOR :
            BSP_GPIO_Write_Pin(CONTACTORS_PORT, ARRAY_CONTACTOR_PIN, state);
            break;
        case ARRAY_PRECHARGE :
            BSP_GPIO_Write_Pin(CONTACTORS_PORT, ARRAY_PRECHARGE_PIN, state);
            break;
        case MOTOR_CONTACTOR :
            BSP_GPIO_Write_Pin(CONTACTORS_PORT, MOTOR_CONTACTOR_PIN, state);
            break;
        default:
            break;
    }
}