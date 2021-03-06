/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "BSP_SPI.h"
#include "stm32f4xx.h"
#include "os.h"
//#include "BSP_OS.h"


#define SPI_PORT SPI3

static void spi_post(void) {

}

static void spi_pend(void) {

}

// Use this inline function to wait until SPI communication is complete
static inline void SPI_Wait(){
	spi_pend();
}

/** SPI_WriteRead
 * @brief   Sends and receives a byte of data on the SPI line.
 * @param   txData single byte that will be sent to the device.
 * @return  rxData single byte that was read from the device.
 */
static uint8_t SPI_WriteRead(uint8_t txData){

	SPI_TypeDef *bus = SPI_PORT;
	
	SPI_Wait(bus);
	bus->DR = txData & 0x00FF;
	SPI_Wait(bus);
	return bus->DR & 0x00FF;
}


/**
 * @brief   Initializes the SPI port.
 * @return  None
 */
void BSP_SPI_Init(void) {

    GPIO_InitTypeDef GPIO_InitStruct;
	SPI_InitTypeDef SPI_InitStruct;


    
	//      SPI configuration:
	//          speed : 125kbps
	//          CPOL : 1 (polarity of clock during idle is high)
	//          CPHA : 1 (tx recorded during 2nd edge)
	// Pins:
	//      SPI3:
	//          PA5 : SCK
	//          PA6 : MISO
	//          PA7 : MOSI 
	//          PA4 : CS
	
	// Initialize clocks
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);
	
	// Initialize pins
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_PinAFConfig(GPIOC, GPIO_PinSource5, GPIO_AF_SPI3);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_SPI3);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_SPI3);
	
	// Initialize SPI port
	SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
	SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStruct.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStruct.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
	SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStruct.SPI_CRCPolynomial = 0;	
	SPI_Init(SPI3, &SPI_InitStruct);
	SPI_Cmd(SPI3, ENABLE);

	// Initialize CS pin
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOA, &GPIO_InitStruct);

	//Configure SPI3 interrupt priority
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel = SPI3_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);

	//Enable the Rx buffer not empty interrupt
	SPI_I2S_ITConfig(SPI3, SPI_I2S_IT_RXNE, ENABLE);
}

/**
 * @brief   Transmits data to through SPI.
 * @note    Blocking statement
 * @param   txBuf   data array that contains the data to be sent.
 * @param   txLen   length of data array.
 * @return  None
 */
void BSP_SPI_Write(uint8_t *txBuf, uint32_t txLen) {
    for(uint32_t i = 0; i < txLen; i++){
		SPI_WriteRead(SPI_PORT, txBuf[i]);
	}
}

/**
 * @brief   Gets the data from SPI.
 * @note    Blocking statement
 * @param   rxBuf   data array to store the data that is received.
 * @param   rxLen   length of data array.
 * @return  None
 */
void BSP_SPI_Read(spi_port_t port, uint8_t *rxBuf, uint32_t rxLen) {
    for(uint32_t i = 0; i < rxLen; i++){
		rxBuf[i] = SPI_WriteRead(SPI_PORT, 0x00);
	}

}


void SPI3_Handler(){
	// Save the CPU registers
	CPU_SR_ALLOC();

	// Protect a critical section
	CPU_CRITICAL_ENTER();

	// make the kernel aware that the interrupt has started
	OSIntEnter();
	CPU_CRITICAL_EXIT();
	spi_post();
	
	//make the kernel aware that the interrupt has ended
	OSIntExit();
}



