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

#ifndef BSP_UART_H
#define BSP_UART_H

#include "common.h"

typedef enum { kUart2, kUart3, kNumUart } Uart;

/**
 * @brief   Initializes the UART peripheral
 */
void BspUartInit(Uart);

/**
 * @brief   Gets one line of ASCII text that was received
 *          from a specified UART device
 * @pre     str should be at least 128bytes long.
 * @param   uart device selected
 * @param   str pointer to buffer string
 * @return  number of bytes that was read
 */
uint32_t BspUartRead(Uart uart, char *str);

/**
 * @brief   Transmits data to through a specific
 *          UART device (represented as a line of data
 *          in csv file).
 * @param   uart device selected
 * @param   str pointer to buffer with data to send.
 * @param   len size of buffer
 * @return  number of bytes that were sent
 */
uint32_t BspUartWrite(Uart uart, char *str, uint32_t len);

#endif

/* @} */
