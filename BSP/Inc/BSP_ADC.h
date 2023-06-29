/* Copyright (c) 2020 UT Longhorn Racing Solar */

/** @defgroup BSP_ADC */
/** @ingroup BSP_ADC 
 * @{ */

/**
 * Header file for the library to interact
 * with the Analog to Digital Converter (ADC)
 */

#ifndef __BSP_ADC_H
#define __BSP_ADC_H

#include "bsp.h"
#include "common.h"
#include "config.h"

#define MAX_CHANNELS 10
#define ADC_PRECISION_BITS 12
#define ADC_RANGE_MILLIVOLTS 3300

typedef enum 
{
    Accelerator_ADC, 
    Brake_ADC,
    NUMBER_OF_CHANNELS
} ADC_t;

/**
 * @brief   Initialize the ADC module
 * @return  None
 */ 
void BSP_ADC_Init(void);

/**
 * @brief   Provides the ADC value of the channel at the specified index
 * @param   hardwareDevice pedal enum that represents the specific device
 * @return  Raw ADC value without conversion
 */ 
int16_t BSP_ADC_Get_Value(ADC_t hardwareDevice);

/**
 * @brief   Provides the ADC value in millivolts of the channel at the specified index
 * @param   hardwareDevice pedal enum that represents the specific device
 * @return  ADC value in millivolts
 */ 
int16_t BSP_ADC_Get_Millivoltage(ADC_t hardwareDevice);

#endif

/*@}*/
