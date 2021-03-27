/* Copyright (c) 2021 UT Longhorn Racing Solar */

#include "BSP_GPIO.h"
#include "stm32f4xx.h"

OS_SEM GPIO_Update_Sem4;

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

static void gpio_pend(void) {
	OS_ERR err;
	CPU_TS ts;
	OSSemPend(&GPIO_Update_Sem4, 0, OS_OPT_PEND_BLOCKING, &ts, &err);
	// TODO: error handling
}

// Use this inline function to wait until GPIO communication is complete
static inline void GPIO_Wait(){
	gpio_pend();
}

static uint8_t GPIO_WriteRead(port_t port, uint16_t data){

	GPIOI_TypeDef *gpio_port = port;
	
	GPIO_Wait(gpio_port);
	gpio_port->DR = data & 0x00FFFF;
	GPIO_Wait(gpio_port);
	return gpio_port->DR & 0x00FFFF;
}

/**
 * @brief   Reads value of the specified port
 * @param   port to read
 * @return  data of the port
 */ 
uint16_t BSP_GPIO_Read(port_t port){
	uint16_t = GPIO_WriteRead(port, 0x0000);
}

/**
 * @brief   Writes data to a specified port 
 * @param   port to write to
 * @param   data to write 
 * @return  None
 */ 
void BSP_GPIO_Write(port_t port, uint16_t data){
	GPIO_WriteRead(port, data);
}