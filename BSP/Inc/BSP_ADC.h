/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file BSP_ADC.h
 * @brief Header file for the library to interact
 * with the Analog to Digital Converter (ADC)
 *
 * @defgroup BSP_ADC
 * @addtogroup BSP_ADC
 * @{
 */

#ifndef BSP_ADC_H
#define BSP_ADC_H

#include "common.h"

#define BSP_ADC_PRECISION_BITS 12

typedef enum { kCh10, kCh11, kNumCh } Adc;

/**
 * @brief   Initialize the ADC module
 * @return  None
 */
void BspAdcInit(void);

/**
 * @brief   Provides the ADC value of the channel at the specified index
 * @param   hardwareDevice pedal enum that represents the specific device
 * @return  Raw ADC value without conversion
 */
int16_t BspAdcGetValue(Adc hardware_device);

/**
 * @brief   Provides the ADC value in millivolts of the channel at the specified
 * index
 * @param   hardwareDevice pedal enum that represents the specific device
 * @return  ADC value in millivolts
 */
int16_t BspAdcGetMillivoltage(Adc hardware_device);

#endif

/* @} */
