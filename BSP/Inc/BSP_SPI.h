/* Copyright (c) 2020 UT Longhorn Racing Solar */

/**
 * Header file for the library to interact
 * over SPI with minion board(s)
 */

#ifndef __BSP_SPI_H
#define __BSP_SPI_H

#include "common.h"
#include "bsp.h"

typedef struct _spi_message {
    int8_t opcode;
    int8_t requestedPort;
    int8_t data;
} spi_message;


// Opcodes
#define SPI_OPCODE_R    0x41
#define SPI_OPCODE_W    0x40

// Register Addresses (BANK = 1)
#define SPI_IODIRA      0x00
#define SPI_IOPOLA      0x01
#define SPI_GPINTENA    0x02
#define SPI_DEFVALA     0x03
#define SPI_INTCONA     0x04
#define SPI_IOCON       0x05
#define SPI_GPPUA       0x06
#define SPI_INTFA       0x07
#define SPI_INTCAPA     0x08
#define SPI_GPIOA       0x09

#define SPI_IODIRB      0x10
#define SPI_IOPOLB      0x11
#define SPI_GPINTENB    0x12
#define SPI_DEFVALB     0x13
#define SPI_INTCONB     0x14
#define SPI_GPPUAB      0x16
#define SPI_INTFAB      0x17
#define SPI_INTCAPB     0x18
#define SPI_GPIOB       0x19
#define SPI_OLATB       0x1A

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
 */
void BSP_SPI_Write(uint8_t* txBuf, uint8_t txLen);

/**
 * @brief   Receives a message through SPI
 * @param   rxBuf buffer to store the received data
 * @param   rxLen length of the buffer
 * @return  None
 */
void BSP_SPI_Read(uint8_t* rxBuf, uint8_t rxLen);

#endif