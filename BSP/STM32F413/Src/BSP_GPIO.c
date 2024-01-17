/* Copyright (c) 2021 UT Longhorn Racing Solar */

#include "BSP_GPIO.h"

#include "Tasks.h"
#include "common.h"

#define NUM_PORTS 4

static GPIO_TypeDef *gpioGetPort(Port port) {
    const GPIO_TypeDef *gpio_mapping[NUM_PORTS] = {GPIOA, GPIOB, GPIOC, GPIOD};

    return (GPIO_TypeDef *)gpio_mapping[port];
}

/**
 * @brief   Initializes a GPIO port
 * @param   port - port to initialize
 * @param	mask - pins
 * @param	direction - input or output
 * @return  None
 */

void BspGpioInit(Port port, uint16_t mask, Direction direction) {
    GPIO_InitTypeDef gpio_init_struct;

    RCC_AHB1PeriphClockCmd(1 << port, ENABLE);
    // Configure the pins to be generic GPIO
    gpio_init_struct.GPIO_Pin = mask;
    gpio_init_struct.GPIO_Mode = direction ? GPIO_Mode_OUT : GPIO_Mode_IN;
    gpio_init_struct.GPIO_OType = GPIO_OType_PP;
    gpio_init_struct.GPIO_Speed = GPIO_Low_Speed;
    gpio_init_struct.GPIO_PuPd = GPIO_PuPd_NOPULL;  // TODO: verify

    // Compute the offset for the port handle from the port passed in
    GPIO_TypeDef *port_handle = gpioGetPort(port);

    // Initialize the GPIO
    GPIO_Init(port_handle, &gpio_init_struct);
}

/**
 * @brief   Reads value of the specified port
 * @param   port to read
 * @return  data of the port
 */

uint16_t BspGpioRead(Port port) {
    GPIO_TypeDef *gpio_port = gpioGetPort(port);

    return GPIO_ReadInputData(gpio_port);
}

/**
 * @brief   Writes data to a specified port
 * @param   port port to write to
 * @param   data data to write
 * @return  None
 */

void BspGpioWrite(Port port, uint16_t data) {
    GPIO_TypeDef *gpio_port = gpioGetPort(port);

    GPIO_Write(gpio_port, data);
}

/**
 * @brief   Reads data from a specified input pin (not applicable to output
 * pins)
 * @param   port The port to read from
 * @param   pin The pin to read from
 * @return  State of the pin
 */

uint8_t BspGpioReadPin(Port port, uint16_t pin_mask) {
    GPIO_TypeDef *gpio_port = gpioGetPort(port);

    return GPIO_ReadInputDataBit(gpio_port, pin_mask);
}

/**
 * @brief   Writes data to a specified pin
 * @param   port The port to write to
 * @param   pin The pin to write to
 * @param   state true=ON or false=OFF
 * @return  None
 */

void BspGpioWritePin(Port port, uint16_t pin_mask, bool state) {
    GPIO_TypeDef *gpio_port = gpioGetPort(port);
    GPIO_WriteBit(gpio_port, pin_mask, (state == ON) ? Bit_SET : Bit_RESET);
}

/**

 * @brief   Returns state of output pin (not applicable to input pins)
 * @param   port The port to get state from
 * @param   pin The pin to get state from
 * @return  1 if pin is high, 0 if low
 */

uint8_t BspGpioGetState(Port port, uint16_t pin) {
    GPIO_TypeDef *gpio_port = gpioGetPort(port);

    return GPIO_ReadOutputDataBit(gpio_port, pin);
}
