/**
 * Header file for the library to interact
 * with the UART2 line
 */

#ifndef __BSP_UART2_H
#define __BSP_UART2_H

#include "common.h"
#include <bsp.h>

/**
 * @brief   Initializes the UART peripheral
 */
void BSP_UART2_Init(void);

/**
 * @brief   Gets one line of ASCII text that was received.
 * @pre     str should be at least 128bytes long.
 * @param   str pointer to buffer string
 * @return  number of bytes that was read
 */
uint32_t BSP_UART2_Read(char *str);

/**
 * @brief   Transmits data to through UART line
 * @param   str pointer to buffer with data to send.
 * @param   len size of buffer
 * @return  number of bytes that were sent
 */
uint32_t BSP_UART2_Write(char *str, uint32_t len);

#endif
