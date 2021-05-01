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
 * @return  None
 */ 
void BSP_GPIO_Init(port_t port, uint16_t mask, uint8_t write);

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
 * @brief   Reads data to a specified pin
 * @param   port The port to read from
 * @param   pin The pin to read from 
 * @return  State of the pin
 */ 
uint8_t BSP_GPIO_Read_Pin(port_t port, uint8_t pin);

/**
 * @brief   Writes data to a specified pin
 * @param   port The port to write to
 * @param   pin The pin to write to 
 * @param   enable output of pin
 * @return  None
 */ 
void BSP_GPIO_Write_Pin(port_t port, uint8_t pin, uint8_t enable);

/**
 * @brief   Returns state of output pin
 * @param   port The port to get state from
 * @param   pin The pin to get state from
 * @return  1 if pin is high, 0 if low
 */ 
uint8_t BSP_GPIO_Get_State(port_t port, uint8_t pin);

#endif
