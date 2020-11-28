/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "BSP_Precharge.h"

#define FILE_NAME DATA_PATH(PRECHARGE_CSV)

/**
 * @brief writes the status (ON/OFF) of a board to a CSV file
 * 
 * @param board to get status (Array | Motor)
 * port to write to (A|B|C)
 * status of board (ON | OFF)
 * 
 * @return none
*/
void BSP_Precharge_Write(board_t board, port_t port, State status){
    // first get previous board states from csv file
    int data = BSP_GPIO_Read(port);
    
    // update bit of data accordingly

    int mask = (1 << board);
    data = data & (~mask);
    data = data | (status << board);

    BSP_GPIO_Write(port, data);
}






