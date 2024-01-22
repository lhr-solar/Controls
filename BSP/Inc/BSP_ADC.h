/**
 * @file BSP_ADC.h
 * @brief This module provides a low-level interface to two ADC channels, 
 * which are intended to be used for the accelerator and brake pedals.
 * 
 */

#ifndef BSP_ADC_H
#define BSP_ADC_H

#include "common.h"

/**
 * @brief This macro represents the maximum number of ADC channels that can be used.
*/
#define MAX_CHANNELS 10

/**
 * @brief This macro represents the number of bits of precision of the ADC.
*/
#define ADC_PRECISION_BITS 12

/**
 * @brief This macro represents the range of the ADC in millivolts.
*/
#define ADC_RANGE_MILLIVOLTS 3300

/**
 * @brief This enum represents the ADC channels that are available on the board.
*/

typedef enum { kCh10, kCh11, kNumCh } Adc;

/**
 * @brief   Initialize the ADC module
 * @param   None
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



