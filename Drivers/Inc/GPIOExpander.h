/* Copyright (c) 2020 UT Longhorn Racing Solar */


#ifndef __GPIOEXPANDER_H
#define __GPIOEXPANDER_H

#include "common.h"

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

#endif
