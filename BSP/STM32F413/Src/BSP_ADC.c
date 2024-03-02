/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "BSP_ADC.h"

#include "stm32f4xx.h"

#define NUM_ADC 2
#define DMA_BUFFER_SIZE 2
#define ADC10_RANK 1
#define ADC11_RANK 2

#define ADC_RANGE_MILLIVOLTS 3300

static volatile uint16_t adc_results[NUM_ADC];

static void adcInitDma(void) {
    // Start the clock for the DMA
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

    // Create the DMA structure
    DMA_InitTypeDef dma_init_struct;

    dma_init_struct.DMA_Channel = DMA_Channel_0;
    dma_init_struct.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
    dma_init_struct.DMA_Memory0BaseAddr = (uint32_t)&adc_results;
    dma_init_struct.DMA_DIR = DMA_DIR_PeripheralToMemory;
    dma_init_struct.DMA_BufferSize = DMA_BUFFER_SIZE;
    dma_init_struct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    dma_init_struct.DMA_MemoryInc = DMA_MemoryInc_Enable;
    dma_init_struct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    dma_init_struct.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    dma_init_struct.DMA_Mode = DMA_Mode_Circular;
    dma_init_struct.DMA_Priority = DMA_Priority_High;
    dma_init_struct.DMA_FIFOMode = DMA_FIFOMode_Disable;
    dma_init_struct.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
    dma_init_struct.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    dma_init_struct.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_Init(DMA2_Stream0, &dma_init_struct);

    // Enable DMA2 stream 0
    DMA_Cmd(DMA2_Stream0, ENABLE);
}

/**
 * @brief   Initializes the ADC module. This is to measure the hall effect
 * sensors on the Current Monitor Board.
 * @param   None
 * @return  None
 */
void BspAdcInit(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,
                           ENABLE);  // Enable the ADC clock
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,
                           ENABLE);  // Enable the PC clock for port C

    adcInitDma();

    GPIO_InitTypeDef gpio_init_struct;
    gpio_init_struct.GPIO_Pin =
        GPIO_Pin_0 | GPIO_Pin_1;                    // Using pin PC0 and PC1
    gpio_init_struct.GPIO_Mode = GPIO_Mode_AN;      // Analog Input
    gpio_init_struct.GPIO_PuPd = GPIO_PuPd_NOPULL;  // High impedence
    GPIO_Init(GPIOC, &gpio_init_struct);

    // ADC Common Init
    ADC_CommonInitTypeDef adc_common_struct;
    adc_common_struct.ADC_Mode = ADC_Mode_Independent;
    adc_common_struct.ADC_Prescaler = ADC_Prescaler_Div2;
    adc_common_struct.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
    adc_common_struct.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
    ADC_CommonInit(&adc_common_struct);

    // Set up to use DMA so that multiple channels can be used
    ADC_InitTypeDef adc_init_struct;  // Initialization structure
    adc_init_struct.ADC_Resolution = ADC_Resolution_12b;  // High resolution
    adc_init_struct.ADC_ScanConvMode =
        ENABLE;  // So we can go through all the channels
    adc_init_struct.ADC_ContinuousConvMode = ENABLE;  // Cycle the channels
    adc_init_struct.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
    adc_init_struct.ADC_ExternalTrigConv = DISABLE;
    adc_init_struct.ADC_DataAlign = ADC_DataAlign_Right;
    adc_init_struct.ADC_NbrOfConversion =
        NUM_ADC;  // We have two channels that we need to read

    ADC_Init(ADC1, &adc_init_struct);

    // Configure the channels
    // Apparently channel 2 has priority, or is at least read first.
    // If you change the priorities, be prepared to have the order in the array
    // change.
    ADC_RegularChannelConfig(ADC1, ADC_Channel_10, ADC10_RANK,
                             ADC_SampleTime_480Cycles);  // Accelerator
    ADC_RegularChannelConfig(ADC1, ADC_Channel_11, ADC11_RANK,
                             ADC_SampleTime_480Cycles);  // Brake

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
int16_t BspAdcGetValue(Adc hardware_device) {
    // Get ADC raw data
    uint16_t data = adc_results[hardware_device];

    return (int16_t)data;
}

/**
 * @brief   Gets converted ADC value in units of mV.
 * @param   None
 * @return  millivoltage value ADC measurement
 */
int16_t BspAdcGetMillivoltage(Adc hardware_device) {
    // Get ADC raw data
    int16_t data = (int16_t)adc_results[hardware_device];

    // Convert to millivoltage
    return (int16_t)((ADC_RANGE_MILLIVOLTS * data) >> BSP_ADC_PRECISION_BITS);
}
