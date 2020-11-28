/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "Precharge.h"

#define FILE_NAME DATA_PATH(PRECHARGE_CSV)
#define PRECHARGE_PORT PORTC
#define MOTOR_BIT 3

/**
 * @brief writes the status (ON/OFF) of a board to a CSV file
 * 
 * @param board to get status (Array | Motor)
 * status of board (ON | OFF)
 * 
 * @return none
*/
void Precharge_Write(board_t board, State status){
    // first get previous board states from csv file
    int data = BSP_GPIO_Read(PRECHARGE_PORT);
    
    // update bit of data accordingly
    if(board == MOTOR_PRECHARGE){
        board = MOTOR_BIT;
    }

    int mask = (1 << board);
    data = data & (~mask);
    data = data | (status << board);

    BSP_GPIO_Write(PRECHARGE_PORT, data);
}

/**
 * @brief reads the data from the Precharge port
 * 
 * @param board to get status (Array | Motor)
 * 
 * @return status of board
*/
State Precharge_Read(board_t board){
    if(board == ARRAY_PRECHARGE){
        return(BSP_GPIO_Read(PRECHARGE_PORT) & 0x1);
    }
    else{
        return((BSP_GPIO_Read(PRECHARGE_PORT) & 0x8) >> 3);
    }
}






