/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file GPIOExpander.h
 * @brief
 *
 * @defgroup GPIOExpander
 * @addtogroup GPIOExpander
 * @{
 */

/* Deprecated */

#ifndef __GPIOEXPANDER_H
#define __GPIOEXPANDER_H

#include "common.h"

typedef struct _spi_message {
  int8_t opcode;
  int8_t requestedPort;
  int8_t data;
} spi_message;

// Opcodes
#define SPI_OPCODE_R 0x41
#define SPI_OPCODE_W 0x40

// Register Addresses (BANK = 0)
#define SPI_IODIRA 0x00
#define SPI_IOPOLA 0x02
#define SPI_GPINTENA 0x04
#define SPI_DEFVALA 0x06
#define SPI_INTCONA 0x08
#define SPI_IOCONA 0x0A
#define SPI_GPPUA 0x0C
#define SPI_INTFA 0x0E
#define SPI_INTCAPA 0x10
#define SPI_GPIOA 0x12

#define SPI_IODIRB 0x01
#define SPI_IOPOLB 0x03
#define SPI_GPINTENB 0x05
#define SPI_DEFVALB 0x07
#define SPI_INTCONB 0x09
#define SPI_GPPUAB 0x0D
#define SPI_INTFAB 0x0F
#define SPI_INTCAPB 0x11
#define SPI_GPIOB 0x13
#define SPI_OLATB 0x15

#endif

/* @} */
