/* Copyright (c) 2020 UT Longhorn Racing Solar */

#ifndef __PRECHARGE_H
#define __PRECHARGE_H

#include "common.h"
#include "config.h"
#include "BSP_GPIO.h"

#define PRECHARGE_PORT PORTC
#define MOTOR_BIT 3

typedef enum {ARRAY_PRECHARGE = 0, MOTOR_PRECHARGE, NUM_PRECHARGES} board_t;

/**
 * @brief   Initializes precharges to be used
 *          in connection with the Motor and Array
 * @param   board the precharge to initialize
 * @return  None
 */ 
void Precharges_Init(board_t board);

/**
 * @brief writes the status (ON/OFF) of a board to a CSV file
 * 
 * @param board to get status (Array | Motor)
 * status of board (ON | OFF)
 * 
 * @return none
*/
void Precharge_Write(board_t board, State status);

/**
 * @brief reads the data from the Precharge port
 * 
 * @param board to get status (Array | Motor)
 * 
 * @return status of board
*/
State Precharge_Read(board_t board);

#endif
