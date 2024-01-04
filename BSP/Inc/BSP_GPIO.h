/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file BSP_GPIO.h
 * @brief Header file for the library to interact
 * with the GPIO ports
 * 
 * @defgroup BSP_GPIO
 * @addtogroup BSP_GPIO
 * @{
 */

#ifndef __BSP_GPIO_H
#define __BSP_GPIO_H

#include "common.h"
#ifdef MOCKING
#include "fff.h"
#else
#include "config.h"
#include "stm32f4xx_gpio.h"
#include <bsp.h>
#include <stdbool.h>
#endif

typedef enum {PORTA = 0, PORTB, PORTC, PORTD, NUM_PORTS} port_t; 
typedef enum {INPUT = 0, OUTPUT} direction_t;

#ifdef MOCKING
DECLARE_FAKE_VOID_FUNC(BSP_GPIO_Write_Pin, port_t, uint16_t, bool);
DECLARE_FAKE_VOID_FUNC(BSP_GPIO_Init, port_t, uint16_t, direction_t);
//FAKE_VOID_FUNC3(OSMutexCreate, struct os_mutex, CPU_CHAR, OS_ERR);
DECLARE_FAKE_VALUE_FUNC(uint8_t,BSP_GPIO_Get_State, port_t, uint16_t );
//FAKE_VOID_FUNC5(OSMutexPend, OS_MUTEX, OS_TICK, OS_OPT, CPU_TS, OS_ERR);
//FAKE_VOID_FUNC3(OSMutexPost, OS_MUTEX, OS_OPT, OS_ERR);
DECLARE_FAKE_VOID_FUNC(assertOSError, int);
#else

/**
 * @brief   Initializes a GPIO port
 * @param   port - port to initialize
 * @param	mask - pins
 * @param	direction - input or output 
 * @return  None
 */ 
void BSP_GPIO_Init(port_t port, uint16_t mask, direction_t direction);

/**
 * @brief   Reads value of the specified port
 * @param   port to read
 * @return  data of the port
 */ 
uint16_t BSP_GPIO_Read(port_t port);

/**
 * @brief   Writes data to a specified port 
 * @param   port port to write to
 * @param   data data to write 
 * @return  None
 */ 
void BSP_GPIO_Write(port_t port, uint16_t data);

/**
 * @brief   Reads data from a specified pin (not applicalbe to output pins)
 * @param   port The port to read from
 * @param   pinmask Mask from stm header file that says which pin to read from
 * @return  State of the pin
 */ 
uint8_t BSP_GPIO_Read_Pin(port_t port, uint16_t pinmask);

/**
 * @brief   Writes data to a specified pin
 * @param   port The port to write to
 * @param   pinmask Mask from stm header file that says which pin to write too
 * @param   state true=ON or false=OFF
 * @return  None
 */ 
void BSP_GPIO_Write_Pin(port_t port, uint16_t pinmask, bool state);

/**
 * @brief   Returns state of output pin (not applicable to input pins)
 * @param   port The port to get state from
 * @param   pin The pin to get state from
 * @return  1 if pin is high, 0 if low
 */ 
uint8_t BSP_GPIO_Get_State(port_t port, uint16_t pin);

#endif
#endif


/* @} */
