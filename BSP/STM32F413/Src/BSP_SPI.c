/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "BSP_SPI.h"
#include "stm32f4xx.h"
#include "os.h"

#define SPI_PORT SPI3

// These are the sizes of the fifos.
// You can write/read more than this at once,
// but performance will degrade slightly.
#define TX_SIZE 128
#define RX_SIZE 64

#define FIFO_TYPE uint8_t
#define FIFO_SIZE TX_SIZE
#define FIFO_NAME txfifo
#include "fifo.h"

#define FIFO_TYPE uint8_t
#define FIFO_SIZE RX_SIZE
#define FIFO_NAME rxfifo
#include "fifo.h"

static txfifo_t spiTxFifo;
static rxfifo_t spiRxFifo;

static OS_SEM SPI_Update_Sem4;

static void spi_post(void) {
	OS_ERR err;
	OSSemPost(&SPI_Update_Sem4, OS_OPT_POST_1, &err);
	// TODO: error handling
}

static void spi_pend(void) {
	OS_ERR err;
	CPU_TS ts;
	OSSemPend(&SPI_Update_Sem4, 0, OS_OPT_PEND_BLOCKING, &ts, &err);
	// TODO: error handling
}

// Use this inline function to wait until SPI communication is complete
static inline void SPI_Wait(void) {
	while((SPI3->SR & (SPI_SR_TXE | SPI_SR_RXNE)) == 0 || (SPI3->SR & SPI_SR_BSY));
}

// Use this inline function to wait until SPI communication is complete
static inline void SPI_WaitTx(void) {
	SPI_I2S_ITConfig(SPI3, SPI_I2S_IT_TXE, ENABLE);
	spi_pend();
}

/** SPI_WriteRead
 * @brief   Sends and receives a byte of data on the SPI line.
 * @param   txData single byte that will be sent to the device.
 * @return  rxData single byte that was read from the device.
 */
static uint8_t SPI_WriteRead(uint8_t txData){

	SPI_TypeDef *bus = SPI_PORT;
	
	SPI_Wait();
	bus->DR = txData & 0x00FF;
	SPI_Wait();
	return bus->DR & 0x00FF;
}


/**
 * @brief   Initializes the SPI port.
 * @return  None
 */
void BSP_SPI_Init(void) {

	OS_ERR err;
	OSSemCreate(&SPI_Update_Sem4, "SPI Update", 0, &err);

	spiTxFifo = txfifo_new();
	spiRxFifo = rxfifo_new();

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

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI3);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI3);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI3);
	
	// Initialize SPI port
	SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
	SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStruct.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStruct.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
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

	// Configure SPI3 interrupt priority
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel = SPI3_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);

	// Enable the Rx buffer not empty interrupt
	SPI_I2S_ITConfig(SPI3, SPI_I2S_IT_RXNE, ENABLE);
}

/**
 * @brief   Transmits data to through SPI.
 * @note    Blocking statement
 * @param   txBuf   data array that contains the data to be sent.
 * @param   txLen   length of data array.
 * @return  None
 * Do not call from an ISR
 */
void BSP_SPI_Write(uint8_t *txBuf, uint8_t txLen) {
	// If we're below an experimentally-determined value, just use polling
	if(txLen < 8) { 
		for(int i = 0; i < txLen; i++) {
			SPI_WriteRead(txBuf[i]);
		}
	} else {
		// If we have a lot of data, we use interrupts to mitigate it
		// Fill as much of the fifo as possible
		size_t i = 0;
		while(i < txLen) {
			// Put as much data into the fifo as can fit
			while(i < txLen && txfifo_put(&spiTxFifo, txBuf[i])) {
				i++;
			}

			// Wait for the transmission to complete
			SPI_WaitTx();
		}

		SPI_Wait();
	}
}

/**
 * @brief   Gets the data from SPI.
 * @note    Blocking statement
 * @param   rxBuf   data array to store the data that is received.
 * @param   rxLen   length of data array.
 * @return  None
 * Do not call from an ISR
 */
void BSP_SPI_Read(uint8_t *rxBuf, uint8_t rxLen) {

	// If we have only a little amount of data, just use polling
	if(rxLen < 8) {
		for(int i = 0; i < rxLen; i++) {
			rxBuf[i] = SPI_WriteRead(0x00);
		}
	} else {
		SPI_I2S_ITConfig(SPI3, SPI_I2S_IT_RXNE, ENABLE);
		// Fill the fifo with zeros to read
		size_t i = 0, r = 0;
		// Empty the fifo
		rxfifo_renew(&spiRxFifo);
		// Read the data
		while(i < rxLen) {
			// Keep filling the fifo with data until we have read everything
			while(i < rxLen && txfifo_put(&spiTxFifo, 0)) {
				i++;
			}

			// Wait for the transmission to complete
			SPI_WaitTx();

			// Busy wait the last bit, just to ensure all bytes have been received
			SPI_Wait();

			// Copy the data out of the fifo
			while(r < i && rxfifo_get(&spiRxFifo, &rxBuf[r])) {
				r++;
			}
		}
		SPI_I2S_ITConfig(SPI3, SPI_I2S_IT_RXNE, DISABLE);
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
	
	// Handle the interrupts
	if (SPI_I2S_GetITStatus(SPI3, SPI_I2S_IT_TXE) == SET){
		// Check to see if there is any data awaiting transmission
		if(!txfifo_get(&spiTxFifo, (uint8_t*)&SPI3->DR)) {
			// We are out of data, so turn off the interrupt and post the semaphore
			SPI_I2S_ITConfig(SPI3, SPI_I2S_IT_TXE, DISABLE);
			spi_post();
		}
	}
	if (SPI_I2S_GetITStatus(SPI3, SPI_I2S_IT_RXNE) == SET){
		// Get the incoming data, put it in the fifo
		// If this overflows, it's the user's fault.
		rxfifo_put(&spiRxFifo, SPI3->DR);
	}
	
	//make the kernel aware that the interrupt has ended
	OSIntExit();
}



