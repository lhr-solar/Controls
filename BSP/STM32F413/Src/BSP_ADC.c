/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "BSP_ADC.h"
#include "stm32f4xx.h"

static volatile uint16_t ADCresults[2];

static void ADC_InitDMA(void) {
	// Start the clock for the DMA
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

	// Create the DMA structure
	DMA_InitTypeDef DMA_InitStruct;

	DMA_InitStruct.DMA_Channel = DMA_Channel_0;
	DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&(ADC1->DR);
	DMA_InitStruct.DMA_Memory0BaseAddr = (uint32_t) &ADCresults;
	DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStruct.DMA_BufferSize = 2;
	DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStruct.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStruct.DMA_Priority = DMA_Priority_High;
	DMA_InitStruct.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStruct.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	DMA_InitStruct.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStruct.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA2_Stream0, &DMA_InitStruct);

	// Enable DMA2 stream 0
	DMA_Cmd(DMA2_Stream0, ENABLE);
}

/**
 * @brief   Initializes the ADC module. This is to measure the hall effect sensors
 *          on the Current Monitor Board.
 * @param   None
 * @return  None
 */
void BSP_ADC_Init(ADC_t ADC_Channel) {
    (void) ADC_Channel; // Initialization is the same for both
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);	// Enable the ADC clock
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);	// Enable the PA clock

	ADC_InitDMA();

	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_2;	// Using pins PA2 and PA3 for the ADC
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AN;						// Analog Input
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;				// High impedence
	GPIO_Init(GPIOA,&GPIO_InitStruct);

	// ADC Common Init
	ADC_CommonInitTypeDef ADC_CommonStruct;
	ADC_CommonStruct.ADC_Mode = ADC_Mode_Independent;
	ADC_CommonStruct.ADC_Prescaler = ADC_Prescaler_Div2;
	ADC_CommonStruct.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
	ADC_CommonStruct.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
	ADC_CommonInit(&ADC_CommonStruct);

	// Set up to use DMA so that multiple channels can be used
	ADC_InitTypeDef ADC_InitStruct;	// Initialization structure
	ADC_InitStruct.ADC_Resolution = ADC_Resolution_12b;	// High resolution
	ADC_InitStruct.ADC_ScanConvMode = ENABLE;						// So we can go through all the channels
	ADC_InitStruct.ADC_ContinuousConvMode = ENABLE; 		// Cycle the channels
	ADC_InitStruct.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	ADC_InitStruct.ADC_ExternalTrigConv = DISABLE;
	ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStruct.ADC_NbrOfConversion = 2;							// We have two channels that we need to read

	ADC_Init(ADC1, &ADC_InitStruct);

	// Configure the channels
	// Apparently channel 2 has priority, or is at least read first.
	// If you change the priorities, be prepared to have the order in the array change.
	ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 2, ADC_SampleTime_480Cycles);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 1, ADC_SampleTime_480Cycles);

	ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);

	ADC_DMACmd(ADC1, ENABLE);

	// Enable ADC1
	ADC_Cmd(ADC1, ENABLE);

	ADC_SoftwareStartConv(ADC1);
}

/**
 * @brief   Gets converted ADC value in units of mV.
 * @param   None
 * @return  millivoltage value ADC measurement
 */
int16_t BSP_ADC_High_GBSP_ADC_Get_Value(ADC_t hardwareDevice) {

    // Get ADC raw data
    uint16_t data = ADCresults[hardwareDevice];
    
    return (int16_t) data;
}

/**
 * @brief   Gets converted ADC value in units of mV.
 * @param   None
 * @return  millivoltage value ADC measurement
 */
int16_t BSP_ADC_Get_Millivoltage(ADC_t hardwareDevice) {

    // Get ADC raw data
    int16_t data = (int16_t) ADCresults[hardwareDevice];
    
    // Convert to millivoltage
    return (ADC_RANGE_MILLIVOLTS * data) >> ADC_PRECISION_BITS;
}