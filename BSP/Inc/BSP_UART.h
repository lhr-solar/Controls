/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file BSP_USART.h
 * @brief Header file for the library to interact
 * with the USART line
 *
 * @defgroup BSP_USART
 * @addtogroup BSP_USART
 * @{
 */

#ifndef BSP_UART_H
#define BSP_UART_H

#include "common.h"

typedef enum { kUart2, kUart3, kNumUart } Uart;

/**
 * @brief   Initializes the USART peripheral
 */
void BspUartInit(Uart);

/**
 * @brief   Gets one line of ASCII text that was received
 *          from a specified USART device
 * @pre     str should be at least 128bytes long.
 * @param   usart device selected
 * @param   str pointer to buffer string
 * @return  number of bytes that was read
 */
uint32_t BspUartRead(Uart usart, char *str);

/**
 * @brief   Transmits data to through a specific
 *          USART device (represented as a line of data
 *          in csv file).
 * @param   usart device selected
 * @param   str pointer to buffer with data to send.
 * @param   len size of buffer
 * @return  number of bytes that were sent
 */
uint32_t BspUartWrite(Uart usart, char *str, uint32_t len);

#endif

/* @} */
