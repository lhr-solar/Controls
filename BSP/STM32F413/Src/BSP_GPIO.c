/* Copyright (c) 2021 UT Longhorn Racing Solar */

#include "BSP_GPIO.h"
#include "stm32f4xx.h"
#include "Tasks.h"

OS_SEM GPIO_Update_Sem4;

/**
 * @brief   Initializes a GPIO port
 * @param   port to initialize
 * @return  None
 */ 
void BSP_GPIO_Init(port_t port, uint16_t mask, uint8_t write){
	/*
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
	*/
	
	GPIO_InitTypeDef GPIO_InitStruct;

	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;	
	GPIO_InitStruct.GPIO_PuPd =  GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	
		switch(port){
			case PORTA:
				// Port used by Switches 
				// Pin 0 to 7 are inputs
				// THIS MIGHT NOT BE NEEDED BECAUSE IT MIGHT BE INIT IN BSP_SPI
				RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
				GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
				GPIO_InitStruct.GPIO_Mode =  GPIO_Mode_IN;
				GPIO_Init(GPIOA, &GPIO_InitStruct);
				break;
			case PORTB:
				// Port used by Switches
				// Pin 7 is input	
				// THIS MIGHT NOT BE NEEDED BECAUSE IT MIGHT BE INIT IN BSP_SPI
				RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
				GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7;
				GPIO_InitStruct.GPIO_Mode =  GPIO_Mode_IN;
				GPIO_Init(GPIOB, &GPIO_InitStruct);
				break;
			case PORTC:
				// Port used by Lights, Contactors and Precharge
				// All pins are outputs
				RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
				GPIO_InitStruct.GPIO_Pin = GPIO_Pin_All;
				GPIO_InitStruct.GPIO_Mode =  GPIO_Mode_OUT;
				GPIO_Init(GPIOC, &GPIO_InitStruct);
				break;
			case PORTD:
				// Not used (I think)
				RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
				GPIO_Init(GPIOD, &GPIO_InitStruct);
				break;
			default:
				// Don't init any port 
				break;
		}
	

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




