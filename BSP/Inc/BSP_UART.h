#ifndef __BSP_UART_H
#define __BSP_UART_H

#include "common.h"
#include <bsp.h>

/**
 * @brief   Initializes the UART peripheral
 */
void BSP_UART_Init(void);

/**
 * @brief   Gets one line of ASCII text that was received.
 * @pre     str should be at least 128bytes long.
 * @param   str pointer to string to store the string
 * @return  number of bytes that was read
 */
uint32_t BSP_UART_Read(char *str);

/**
 * @brief   Transmits data to through UART line
 * @param   str : pointer to buffer with data to send.
 * @param   len : size of buffer
 * @return  number of bytes that were sent
 */
uint32_t BSP_UART_Write(char *str, uint32_t len);

#endif
