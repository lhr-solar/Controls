/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file BSP_SPI.h
 * @brief Header file for the library to interact
 * over SPI with minion board(s)
 * 
 * @addtogroup BSP_SPI
 * @{
 */

#ifndef __BSP_SPI_H
#define __BSP_SPI_H

#include "common.h"
#include "bsp.h"
#include "os.h"



/**
 * @brief   Initializes the SPI communication
 *          to be used to communicate with 
 *          minion board(s)
 * @param   None
 * @return  None
 */
void BSP_SPI_Init(void);

/**
 * @brief   Transmits a message through SPI
 * @param   txBuf data to transmit
 * @param   txLen length of the data packet
 * @return  None
 * Do not call from an ISR
 */
void BSP_SPI_Write(uint8_t* txBuf, uint8_t txLen);

/**
 * @brief   Receives a message through SPI
 * @param   rxBuf buffer to store the received data
 * @param   rxLen length of the buffer
 * @return  None
 * Do not call from an ISR
 */
void BSP_SPI_Read(uint8_t* rxBuf, uint8_t rxLen);

#endif


/** @} */
