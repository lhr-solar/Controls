/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file BSP_UART.h
 * @brief Header file for the library to interact
 * with the UART line
 * 
 * @defgroup BSP_UART
 * @addtogroup BSP_UART
 * @{
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
 * @pre     str should be at least 128 bytes long.
 * @param   usart device selected
 * @param   str pointer to buffer string
 * @return  number of bytes that was read
 * 
 * @note This function uses a fifo to buffer the write. If that
 *       fifo is full, this function may block while waiting for
 *       space to open up. Do not call from timing-critical
 *       sections of code.
 */
uint32_t BSP_UART_Read(UART_t usart, char *str);

/**
 * @brief   Transmits data to through a specific 
 *          UART device (represented as a line of data 
 *          in csv file).
 * @param   uart device selected
 * @param   str pointer to buffer with data to send.
 * @param   len size of buffer
 * @return  number of bytes that were sent
 * 
 * @note This function uses a fifo to buffer the write. If that
 *       fifo is full, this function may block while waiting for
 *       space to open up. Do not call from timing-critical
 *       sections of code.
 */
uint32_t BSP_UART_Write(UART_t uart ,char *str, uint32_t len);

#endif


/* @} */
