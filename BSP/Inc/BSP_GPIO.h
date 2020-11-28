/* Copyright (c) 2020 UT Longhorn Racing Solar */

/**
 * Header file for the library to interact
 * with the GPIO ports
 */

#ifndef __BSP_GPIO_H
#define __BSP_GPIO_H

#include "common.h"
#include "config.h"
#include <bsp.h>

typedef enum {PORTA = 0, PORTB, NUM_PORTS} port_t; 

/**
 * @brief   Initializes a GPIO port
 * @param   port to initialize
 * @return  None
 */ 
void BSP_GPIO_Init(port_t port);

/**
 * @brief   Reads value of the specified port
 * @param   port to read
 * @return  data of the port
 */ 
uint16_t BSP_GPIO_Read(port_t port);

/**
 * @brief   Writes data to a specified port 
 * @param   port to write to
 * @param   data to write 
 * @return  None
 */ 
void BSP_GPIO_Write(port_t port, uint16_t data);

#endif
