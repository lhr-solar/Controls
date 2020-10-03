/* Copyright (c) 2020 UT Longhorn Racing Solar */

/**
 * Header file for the library to interact
 * with the UART line
 */

#ifndef __BSP_UART_H
#define __BSP_UART_H

#include "common.h"
#include <bsp.h>

typedef enum {UART_2, UART_3, NUM_UART} UART_t;
/**
 * @brief   Initializes the UART peripheral
 */
void BSP_UART_Init(UART_t);

/**
 * @brief   Gets one line of ASCII text that was received 
 *          from a specified UART device
 * @pre     str should be at least 128bytes long.
 * @param   uart device selected
 * @param   str pointer to buffer string
 * @return  number of bytes that was read
 */
uint32_t BSP_UART_Read(UART_t uart, char *str);

/**
 * @brief   Transmits data to through a specific 
 *          UART device (represented as a line of data 
 *          in csv file).
 * @param   uart device selected
 * @param   str pointer to buffer with data to send.
 * @param   len size of buffer
 * @return  number of bytes that were sent
 */
uint32_t BSP_UART_Write(UART_t uart ,char *str, uint32_t len);

#endif
