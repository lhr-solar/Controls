/* Copyright (c) 2021 UT Longhorn Racing Solar */

#include "BSP_GPIO.h"
#include "Tasks.h"
#include "stm32f4xx.h"

static GPIO_TypeDef* GPIO_GetPort(port_t port){
 	const GPIO_TypeDef* gpio_mapping[4] = {GPIOA, GPIOB, GPIOC, GPIOD};

	return (GPIO_TypeDef *) gpio_mapping[port];

}


/**
 * @brief   Initializes a GPIO port
 * @param   port - port to initialize
 * @param	mask - pins
 * @param	direction - input or output 
 * @return  None
 */ 

void BSP_GPIO_Init(port_t port, uint16_t mask, direction_t direction){
	GPIO_InitTypeDef GPIO_InitStruct;

	RCC_AHB1PeriphClockCmd(1 << port, ENABLE);
	// Configure the pins to be generic GPIO
    GPIO_InitStruct.GPIO_Pin   = mask;
    GPIO_InitStruct.GPIO_Mode  = direction ? GPIO_Mode_OUT : GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Low_Speed;
    GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_NOPULL;  // TODO: verify
    
	// Compute the offset for the port handle from the port passed in
    GPIO_TypeDef *portHandle = GPIO_GetPort(port);

    // Initialize the GPIO
    GPIO_Init(portHandle, &GPIO_InitStruct);
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
 * @param   port port to write to
 * @param   data data to write 
 * @return  None
 */ 

void BSP_GPIO_Write(port_t port, uint16_t data){
	GPIO_TypeDef *gpio_port = GPIO_GetPort(port);

	GPIO_Write(gpio_port, data);
}


/**
 * @brief   Reads data to a specified input pin (not applicable to output pins)
 * @param   port The port to read from
 * @param   pin The pin to read from 
 * @return  State of the pin
 */ 

uint8_t BSP_GPIO_Read_Pin(port_t port, uint8_t pin){
	GPIO_TypeDef *gpio_port = GPIO_GetPort(port);

	return !GPIO_ReadInputDataBit(gpio_port, pin);
}



/**
 * @brief   Writes data to a specified pin
 * @param   port The port to write to
 * @param   pin The pin to write to 
 * @param   state ON or OFF
 * @return  None
 */ 

void BSP_GPIO_Write_Pin(port_t port, uint16_t pin, State state){
	GPIO_TypeDef *gpio_port = GPIO_GetPort(port);
	if(state==ON)
		GPIO_WriteBit(gpio_port, pin, Bit_SET);
	else if(state==OFF)
		GPIO_WriteBit(gpio_port, pin, Bit_RESET);
}



/**

 * @brief   Returns state of output pin (not applicable to input pins)
 * @param   port The port to get state from
 * @param   pin The pin to get state from
 * @return  1 if pin is high, 0 if low
 */ 

uint8_t BSP_GPIO_Get_State(port_t port, uint16_t pin){
	GPIO_TypeDef *gpio_port = GPIO_GetPort(port);

	return !GPIO_ReadOutputDataBit(gpio_port, pin);	
}
