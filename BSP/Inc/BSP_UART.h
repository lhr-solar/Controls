/**
 * @file    BSP_UART.h
 * @brief This module provides a low-level interface to two UART ports,
 * intended for use for the display and USB communication.
 */

#ifndef BSP_UART_H
#define BSP_UART_H

#include "common.h"

/**
 * @brief   Enumeration of the UART devices
 */
typedef enum { kUart2, kUart3, kNumUart } Uart;

/**
 * @brief   Initializes the UART peripheral
 * @param   usart device selected
 */
void BspUartInit(Uart usart);

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
uint32_t BspUartRead(Uart usart, char *str);

/**
 * @brief   Transmits data to through a specific
 *          USART device (represented as a line of data
 *          in csv file).
 * @param   usart device selected
 * @param   str pointer to buffer with data to send.
 * @param   len size of buffer
 * @return  number of bytes that were sent
 *
 * @note This function uses a fifo to buffer the write. If that
 *       fifo is full, this function may block while waiting for
 *       space to open up. Do not call from timing-critical
 *       sections of code.
 */
uint32_t BspUartWrite(Uart usart, char *str, uint32_t len);

#endif