/* Copyright (c) 2020 UT Longhorn Racing Solar */

#ifndef __BSP_PRECHARGE_H
#define __BSP_PRECHARGE_H

#include "common.h"
#include "config.h"
#include <bsp.h>

typedef enum {ARRAY_PRECHARGE = 0, MOTOR_PRECHARGE} board_t;

/**
 * @brief writes the status (ON/OFF) of a board to a CSV file
 * 
 * @param board to get status (Array | Motor)
 * port to write to (A|B|C)
 * status of board (ON | OFF)
 * 
 * @return none
*/
void BSP_Precharge_Write(board_t board, port_t port, State status);

#endif
