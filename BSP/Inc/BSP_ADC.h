#ifndef __BSP_ADC_H
#define __BSP_ADC_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "common.h"
#include "config.h"

#define MAX_CHANNELS 10

typedef enum pedal_t 
{
    Accelerator, 
    Brake
} pedal_t;

/**
 * @brief   Initialize the ADC module
 * @param   None
 * @return  None
 */ 
void BSP_ADC_Init(void);

/**
 * @brief   Provides the ADC value of the channel at the specified index
 * @param   hardwareDevice, index of value in the array of all ADC values obtained with read_ADC_Values
 * @return  rawADCValue
 */ 
int16_t BSP_ADC_Get_Value(pedal_t hardwareDevice);

/**
 * @brief   Provides the ADC value in millivolts of the channel at the specified index
 * @param   indexOfValue, index of value in the array of all ADC values obtained with read_ADC_Values
 * @return  ADC value in millivolts
 */ 
int BSP_ADC_Get_Millivoltage(pedal_t hardwareDevice);

#endif