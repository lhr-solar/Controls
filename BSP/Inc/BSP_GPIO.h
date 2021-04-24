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

typedef enum {PORTA = 0, PORTB, PORTC, PORTD, NUM_PORTS} port_t; 

/**
 * @brief   Initializes a GPIO port
 * @param   port to initialize
 * @param   mask the pins to initialize
 * @param   write whether the pins are outputs (true) or inputs (false)
 * @return  None
 */ 
void BSP_GPIO_Init(port_t port, uint16_t mask, bool write);

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

/**
 * @brief   Reads from a specific pin
 * @param   port The port to read from
 * @param   pin  The pin to read from
 * @return  True if the pin is high
 */
bool BSP_GPIO_Read_Pin(port_t port, uint8_t pin);

/**
 * @brief   Writes data to a specific pin
 * @param   port The port to write to
 * @param   pin  The pin to write to
 * @param   data True if the pin should be set to high
 * @return  None
 */
void BSP_GPIO_Write_Pin(port_t port, uint8_t pin, bool enable);

/**
 * @brief   Returns the state of the output pin
 * @param   port The port to read from
 * @param   pin  The pin, which is configured as an output, to get the state
 * @return  True if the pin is set, false otherwise
 */
bool BSP_GPIO_Get_State(port_t port, uint8_t pin);

#endif
