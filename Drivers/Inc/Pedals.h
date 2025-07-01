/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file Pedals.h
 * @brief Header file for the Pedals driver
 *
 * @defgroup Pedals
 * @addtogroup Pedals
 * @{
 */

#ifndef __PEDALS_H
#define __PEDALS_H

#include "common.h"

/**
 * @brief Stuff
 *
 */
typedef enum { ACCELERATOR, BRAKE, NUMBER_OF_PEDALS } pedal_t;

/**
 * @brief   Initialize the pedals
 * @param   None
 * @return  None
 */
void Pedals_Init(void);

/**
 * @brief   Provides the pedal distance pressed in percetage (accelerator or brake)
 * @param   pedal_t pedal, ACCELERATOR or BRAKE as defined in enum
 * @return  distance the pedal has been pressed in percentage
 */
uint8_t Pedals_Read(pedal_t pedal);

/**
 * @brief   Fetches the millivoltage value of the potentiomenter as provided
 *          by the ADC channel of the requested pedal (Accelerator or Brake),
 * @param   pedal_t, ACCELERATOR or BRAKE as defined in enum
 * @return  Direct analog voltage the pedal outputs
 */
int16_t Pedals_rawVoltage(pedal_t pedal);

#endif

/* @} */
