/* Copyright (c) 2020 UT Longhorn Racing Solar */

#ifndef __PRECHARGE_H
#define __PRECHARGE_H

#include "common.h"
#include "config.h"
#include "BSP_GPIO.h"

#define ARRAY_PRECHARGE_PORT    PORTC
#define ARRAY_PRECHARGE_PIN     10
#define MOTOR_PRECHARGE_PORT    PORTD
#define MOTOR_PRECHARGE_PIN     2
#define MOTOR_BIT               3

typedef enum {ARRAY_PRECHARGE = 0, MOTOR_PRECHARGE, NUM_PRECHARGES} board_t;

/**
 * @brief   initializes the GPIO pins used for precharging
 * 
 * @param   board the precharge to initialize (unused)
 * @return  None
 */
void Precharges_Init(board_t board);

/**
 * @brief writes the status to the board (ON/OFF)
 * 
 * @param board     board to set status (Array/Motor)
 * @param status    status of board (ON/OFF)
 * @return none
*/
void Precharge_Write(board_t board, State status);

/**
 * @brief reads the Precharge port status
 * 
 * @param board board to get status (Array/Motor)
 * @return status of board (ON/OFF)
*/
State Precharge_Read(board_t board);

#endif
