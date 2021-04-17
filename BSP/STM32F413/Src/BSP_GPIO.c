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
void BSP_GPIO_Init(port_t port){
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

static void gpio_pend(void) {
	OS_ERR err;
	CPU_TS ts;
	OSSemPend(&GPIO_Update_Sem4, 0, OS_OPT_PEND_BLOCKING, &ts, &err);
	
	if(err != OS_ERR_NONE){
        printf("OS error code %d\n", err);
    }
}

// Use this inline function to wait until GPIO communication is complete
static inline void GPIO_Wait(){
	gpio_pend();
}

static GPIO_TypeDef* GPIO_GetPort(port_t port){
	GPIO_TypeDef *gpio_port;

	switch(port){
		case PORTA:
		gpio_port = GPIOA;
		break;
		case PORTB:
		gpio_port = GPIOB;
		break;
		case PORTC:
		gpio_port = GPIOC;
		break;
		case PORTD:
		gpio_port = GPIOD;
		break;
		default:
		return;
	}

	return gpio_port;
}

/**
 * @brief   Reads value of the specified port
 * @param   port to read
 * @return  data of the port
 */ 
uint16_t BSP_GPIO_Read(port_t port){
	GPIO_TypeDef *gpio_port = GPIO_GetPort(port);
	
	GPIO_Wait(gpio_port);
	return gpio_port->IDR & 0xFFFF;
}

/**
 * @brief   Writes data to a specified port 
 * @param   port to write to
 * @param   data to write 
 * @return  None
 */ 
void BSP_GPIO_Write(port_t port, uint16_t data){
	GPIO_TypeDef *gpio_port = GPIO_GetPort(port);
	
	GPIO_Wait(gpio_port);
	gpio_port->ODR = data & 0xFFFF;
}