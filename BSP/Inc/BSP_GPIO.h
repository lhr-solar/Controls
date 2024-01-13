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

#ifndef BSP_GPIO_H
#define BSP_GPIO_H

#include "common.h"
#include "stm32f4xx_gpio.h"

typedef enum { kPortA = 0, kPortB, kPortC, kPortD, kNumPorts } Port;
typedef enum { kInput = 0, kOutput } Direction;

/**
 * @brief   Initializes a GPIO port
 * @param   port - port to initialize
 * @param	mask - pins
 * @param	direction - input or output
 * @return  None
 */
void BspGpioInit(Port port, uint16_t mask, Direction direction);

/**
 * @brief   Reads value of the specified port
 * @param   port to read
 * @return  data of the port
 */
uint16_t BspGpioRead(Port port);

/**
 * @brief   Writes data to a specified port
 * @param   port port to write to
 * @param   data data to write
 * @return  None
 */
void BspGpioWrite(Port port, uint16_t data);

/**
 * @brief   Reads data from a specified pin (not applicalbe to output pins)
 * @param   port The port to read from
 * @param   pinmask Mask from stm header file that says which pin to read from
 * @return  State of the pin
 */
uint8_t BspGpioReadPin(Port port, uint16_t pin_mask);

/**
 * @brief   Writes data to a specified pin
 * @param   port The port to write to
 * @param   pinmask Mask from stm header file that says which pin to write too
 * @param   state true=ON or false=OFF
 * @return  None
 */
void BspGpioWritePin(Port port, uint16_t pin_mask, bool state);

/**
 * @brief   Returns state of output pin (not applicable to input pins)
 * @param   port The port to get state from
 * @param   pin The pin to get state from
 * @return  1 if pin is high, 0 if low
 */
uint8_t BspGpioGetState(Port port, uint16_t pin);

#endif

/* @} */
