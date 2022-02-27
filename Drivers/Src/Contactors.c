/* Copyright (c) 2020 UT Longhorn Racing Solar */


#include "Contactors.h"
#include "stm32f4xx.h"

#define CONTACTORS_PORT         PORTC
#define ARRAY_PRECHARGE_PIN_MSK GPIO_Pin_10
#define ARRAY_PRECHARGE_PIN_NUM 10
#define ARRAY_CONTACTOR_PIN_MSK GPIO_Pin_11
#define ARRAY_CONTACTOR_PIN_NUM 11
#define MOTOR_CONTACTOR_PIN_MSK GPIO_Pin_12
#define MOTOR_CONTACTOR_PIN_NUM 12


/**
 * @brief   Initializes contactors to be used
 *          in connection with the Motor and Array
 * @return  None
 */ 
void Contactors_Init() {
    BSP_GPIO_Init(CONTACTORS_PORT, 
                 (ARRAY_CONTACTOR_PIN_MSK) | 
                 (ARRAY_PRECHARGE_PIN_MSK) |
                 (MOTOR_CONTACTOR_PIN_MSK), 
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
            state = BSP_GPIO_Read_Pin(CONTACTORS_PORT, ARRAY_CONTACTOR_PIN_NUM);
            break;
        case ARRAY_PRECHARGE :
            state = BSP_GPIO_Read_Pin(CONTACTORS_PORT, ARRAY_PRECHARGE_PIN_NUM);
            break;
        case MOTOR_CONTACTOR :
            state = BSP_GPIO_Read_Pin(CONTACTORS_PORT, MOTOR_CONTACTOR_PIN_NUM);
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
            BSP_GPIO_Write_Pin(CONTACTORS_PORT, ARRAY_CONTACTOR_PIN_NUM, state);
            break;
        case ARRAY_PRECHARGE :
            BSP_GPIO_Write_Pin(CONTACTORS_PORT, ARRAY_PRECHARGE_PIN_NUM, state);
            break;
        case MOTOR_CONTACTOR :
            BSP_GPIO_Write_Pin(CONTACTORS_PORT, MOTOR_CONTACTOR_PIN_NUM, state);
            break;
        default:
            break;
    }
}