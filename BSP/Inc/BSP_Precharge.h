#ifndef _BSP_PRECHARGE_H_
#define _BSP_PRECHARGE_H_

#include "common.h"
#include "config.h"
#include <bsp.h>

typedef enum {Array = 0, Motor} board_t;

/**
 * @brief writes the status (ON/OFF) of a board to a CSV file
 * 
 * @param board to get status (Array | Motor)
 * status of board (ON | OFF)
 * 
 * @return none
*/
void BSP_Precharge_write(board_t board, state_t status);

#endif