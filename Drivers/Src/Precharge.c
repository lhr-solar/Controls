/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "Precharge.h"

/**
 * @brief   initializes the GPIO pins used for precharging
 * 
 * @param   board the precharge to initialize (unused)
 * @return  None
 */
void Precharges_Init(board_t board) {
    BSP_GPIO_Init(ARRAY_PRECHARGE_PORT,
                  1 << ARRAY_PRECHARGE_PIN,
                  1);
    BSP_GPIO_Init(MOTOR_PRECHARGE_PORT,
                  1 << MOTOR_PRECHARGE_PIN,
                  1);
}

/**
 * @brief writes the status to the board (ON/OFF)
 * 
 * @param board     board to set status (Array/Motor)
 * @param status    status of board (ON/OFF)
 * @return none
*/
void Precharge_Write(board_t board, State status){
    switch (board) {
        case ARRAY_PRECHARGE :
            BSP_GPIO_Write_Pin(ARRAY_PRECHARGE_PORT, 
                               ARRAY_PRECHARGE_PIN, 
                               status);
            break;
        case MOTOR_PRECHARGE :
            BSP_GPIO_Write_Pin(MOTOR_PRECHARGE_PORT, 
                               MOTOR_PRECHARGE_PIN, 
                               status);
            break;
        default :
            break;
    }
}

/**
 * @brief reads the Precharge port status
 * 
 * @param board board to get status (Array/Motor)
 * @return status of board (ON/OFF)
*/
State Precharge_Read(board_t board){
    State status = OFF;
    switch (board) {
        case ARRAY_PRECHARGE :
            status = BSP_GPIO_Read_Pin(ARRAY_PRECHARGE_PORT, 
                                       ARRAY_PRECHARGE_PIN);
            break;
        case MOTOR_PRECHARGE :
            status = BSP_GPIO_Read_Pin(MOTOR_PRECHARGE_PORT, 
                                       MOTOR_PRECHARGE_PIN);
            break;
        default :
            break;
    }
    return status;
}

