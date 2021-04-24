/* Copyright (c) 2021 UT Longhorn Racing Solar */

#include "BSP_GPIO.h"
#include "stm32f4xx.h"

#define GET_HANDLE_FROM_ENUM(x) ((GPIO_TypeDef *) (AHB1PERIPH_BASE + (0x0400 * port)))

/**
 * @brief   Initializes a GPIO port
 * @param   port to initialize
 * @param   mask the pins to initialize
 * @param   write whether the pins are outputs (true) or inputs (false)
 * @return  None
 */
void BSP_GPIO_Init(port_t port, uint16_t mask, bool write) {
    GPIO_InitTypeDef GPIO_InitStruct;

    // Turn on the port clock
    RCC_AHB1PeriphClockCmd(1 << port, ENABLE);

    // Configure the pins to be generic GPIO
    GPIO_InitStruct.GPIO_Pin   = mask;
    GPIO_InitStruct.GPIO_Mode  = write ? GPIO_Mode_OUT : GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Low_Speed;
    GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_NOPULL;  // TODO: verify

    // Compute the offset for the port handle from the port passed in
    GPIO_TypeDef *portHandle = GET_HANDLE_FROM_ENUM(port);

    // Initialize the GPIO
    GPIO_Init(portHandle, &GPIO_InitStruct);
}

/**
 * @brief   Reads value of the specified port
 * @param   port to read
 * @return  data of the port
 * 
 * @note    This function will not work for reading the state of output pins
 */ 
uint16_t BSP_GPIO_Read(port_t port) {
    return GPIO_ReadInputData(GET_HANDLE_FROM_ENUM(port));
}

/**
 * @brief   Writes data to a specified port 
 * @param   port to write to
 * @param   data to write 
 * @return  None
 */ 
void BSP_GPIO_Write(port_t port, uint16_t data) {
    GPIO_Write(GET_HANDLE_FROM_ENUM(port), data);
}

/**
 * @brief   Reads from a specific pin
 * @param   port The port to read from
 * @param   pin  The pin to read from
 * @return  True if the pin is high
 * 
 * @note    This function will not work for reading the state of output pins
 */
bool BSP_GPIO_Read_Pin(port_t port, uint8_t pin) {
    return GPIO_ReadInputDataBit(GET_HANDLE_FROM_ENUM(port), (1 << pin));
}

/**
 * @brief   Writes data to a specific pin
 * @param   port The port to write to
 * @param   pin  The pin to write to
 * @param   data True if the pin should be set to high
 * @return  None
 */
void BSP_GPIO_Write_Pin(port_t port, uint8_t pin, bool enable) {
    GPIO_WriteBit(GET_HANDLE_FROM_ENUM(port), 
                  1 << pin, 
                  enable ? Bit_SET : Bit_RESET);
}

/**
 * @brief   Returns the state of the output pin
 * @param   port The port to read from
 * @param   pin  The pin, which is configured as an output, to get the state
 * @return  True if the pin is set, false otherwise
 */
bool BSP_GPIO_Get_State(port_t port, uint8_t pin) {
    return GPIO_ReadOutputDataBit(GET_HANDLE_FROM_ENUM(port), 1 << pin);
}
