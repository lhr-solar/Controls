/* Copyright (c) 2021 UT Longhorn Racing Solar */

#include "BSP_GPIO.h"
#include "stm32f4xx.h"
#include "Tasks.h"

OS_SEM GPIO_Update_Sem4;

/**
 * @brief   Initializes a GPIO port
 * @param   port -port to initialize, mask - pins, write - input or output
 * @return  None
 */ 
void BSP_GPIO_Init(port_t port, uint16_t mask, uint8_t write){
	
	GPIO_InitTypeDef GPIO_InitStruct;

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

static GPIO_TypeDef* GPIO_GetPort(port_t port){
	const GPIO_TypeDef* gpio_mapping[4] = {GPIOA, GPIOB, GPIOC, GPIOD};

	return gpio_mapping[port];
}

/**
 * @brief   Reads value of the specified port
 * @param   port to read
 * @return  data of the port
 */ 
uint16_t BSP_GPIO_Read(port_t port){
	GPIO_TypeDef *gpio_port = GPIO_GetPort(port);

	return GPIO_ReadInputData(gpio_port);
}

/**
 * @brief   Writes data to a specified port 
 * @param   port to write to
 * @param   data to write 
 * @return  None
 */ 
void BSP_GPIO_Write(port_t port, uint16_t data){
	GPIO_TypeDef *gpio_port = GPIO_GetPort(port);
	
	GPIO_Write(gpio_port, data);
}

/**
 * @brief   Reads data to a specified pin
 * @param   port The port to read from
 * @param   pin The pin to read from 
 * @return  State of the pin
 */ 
uint8_t BSP_GPIO_Read_Pin(port_t port, uint8_t pin){
	GPIO_TypeDef *gpio_port = GPIO_GetPort(port);

	GPIO_ReadInputDataBit(gpio_port, pin);
}

/**
 * @brief   Writes data to a specified pin
 * @param   port The port to write to
 * @param   pin The pin to write to 
 * @param   enable output of pin
 * @return  None
 */ 
void BSP_GPIO_Write_Pin(port_t port, uint8_t pin, uint8_t enable){
	GPIO_TypeDef *gpio_port = GPIO_GetPort(port);

	GPIO_WriteBit(gpio_port, pin, enable);
}

/**
 * @brief   Returns state of output pin
 * @param   port The port to get state from
 * @param   pin The pin to get state from
 * @return  1 if pin is high, 0 if low
 */ 
uint8_t BSP_GPIO_Get_State(port_t port, uint8_t pin){
	GPIO_TypeDef *gpio_port = GPIO_GetPort(port);

	GPIO_ReadOutputDataBit(gpio_port, pin);	
}





