/**
 * @file BSP_GPIO.h
 * @brief This module provides a low-level interface to the Leaderboard's GPIO
 * ports, intended for switches and some lights. The code is fairly
 * straightforward, and only slightly simplifies the
 * [Minion](../Drivers/Minions.html).
 *
 */

#ifndef BSP_GPIO_H
#define BSP_GPIO_H

#include "common.h"
#include "stm32f4xx_gpio.h"

/**
 * @brief GPIO ports
 */
typedef enum { kPortA = 0, kPortB, kPortC, kPortD, kNumPorts } Port;

/**
 * @brief GPIO direction
 */
typedef enum { kInput = 0, kOutput } Direction;

/**
 * @brief   Initializes a GPIO port
 * @param   port port to initialize
 * @param	mask pins
 * @param	direction input or output
 * @return  None
 */
void BspGpioInit(Port port, uint16_t mask, Direction direction);

/**
 * @brief   Reads value of the specified port
 * @param   port port to read
 * @return  data in the port
 */
uint16_t BSP_GPIO_Read(port_t port);

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
