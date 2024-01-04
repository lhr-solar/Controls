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

#ifndef __BSP_ADC_H
#define __BSP_ADC_H
#define MOCK_BSP_ADC
#ifndef MOCK_BSP_ADC
#include "bsp.h"
#endif
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

#ifdef MOCKING
#include "fff.h"
DECLARE_FAKE_VOID_FUNC(BSP_ADC_Init);
DECLARE_FAKE_VALUE_FUNC(int16_t, BSP_ADC_Get_Millivoltage, int);
DECLARE_FAKE_VALUE_FUNC(int16_t, BSP_ADC_Get_Value, int);
#else

/**
 * @brief   Initialize the ADC module
 * @return  None
 */ 
extern void BSP_ADC_Init(void);

/**
 * @brief   Provides the ADC value of the channel at the specified index
 * @param   hardwareDevice pedal enum that represents the specific device
 * @return  Raw ADC value without conversion
 */ 
extern int16_t BSP_ADC_Get_Value(ADC_t hardwareDevice);

/**
 * @brief   Provides the ADC value in millivolts of the channel at the specified index
 * @param   hardwareDevice pedal enum that represents the specific device
 * @return  ADC value in millivolts
 */ 
extern int16_t BSP_ADC_Get_Millivoltage(ADC_t hardwareDevice);

#endif
#endif

/* @} */
