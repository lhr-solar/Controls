/* Copyright (c) 2021 UT Longhorn Racing Solar */

#include "BSP_GPIO.h"
#include "stm32f4xx.h"

/**
 * @brief   Initializes a GPIO port
 * @param   port to initialize
 * @return  None
 */ 
void BSP_GPIO_Init(port_t port){
	GPIO_InitTypeDef GPIO_InitStruct;

		switch(port){
			case PORTA:
				RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
				GPIO_Init(GPIOA, &GPIO_InitStruct);
				break;
			case PORTB:
				RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
				GPIO_Init(GPIOB, &GPIO_InitStruct);
				break;
			case PORTC:
				RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
				GPIO_Init(GPIOC, &GPIO_InitStruct);
				break;
			case PORTD:
				RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
				GPIO_Init(GPIOD, &GPIO_InitStruct);
				break;
			default:
				// Initialize PORTA if argument is not A-D
				RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
				GPIO_Init(GPIOA, &GPIO_InitStruct);

		}

}

/**
 * @brief   Reads value of the specified port
 * @param   port to read
 * @return  data of the port
 */ 
uint16_t BSP_GPIO_Read(port_t port){

}

/**
 * @brief   Writes data to a specified port 
 * @param   port to write to
 * @param   data to write 
 * @return  None
 */ 
void BSP_GPIO_Write(port_t port, uint16_t data){

}