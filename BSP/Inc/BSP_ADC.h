/**
 * @file BSP_ADC.h
 * @brief This module provides a low-level interface to two ADC channels, 
 * which are intended to be used for the accelerator and brake pedals.
 * 
 */

#ifndef __BSP_ADC_H
#define __BSP_ADC_H

#include "bsp.h"
#include "common.h"
#include "config.h"

/**
 * @def MAX_CHANNELS
 * @brief This macro represents the maximum number of ADC channels that can be used.
*/
#define MAX_CHANNELS 10

/**
 * @def ADC_PRECISION_BITS
 * @brief This macro represents the number of bits of precision of the ADC.
*/
#define ADC_PRECISION_BITS 12

/**
 * @def ADC_RANGE_MILLIVOLTS
 * @brief This macro represents the range of the ADC in millivolts.
*/
#define ADC_RANGE_MILLIVOLTS 3300

/**
 * @enum ADC_t
 * @brief This enum represents the ADC channels that are available on the board.
*/
typedef enum {
    Accelerator_ADC, 
    Brake_ADC,
    NUMBER_OF_CHANNELS
} ADC_t;

/**
 * @brief   Initialize the ADC module
 * @param   None
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



