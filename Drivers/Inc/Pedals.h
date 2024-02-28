/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file Pedals.h
 * @brief Header file for the Pedals driver
 *
 * @defgroup Pedals
 * @addtogroup Pedals
 * @{
 */

#ifndef PEDALS_H
#define PEDALS_H

#include "BSP_ADC.h"

/**
 * @brief Stuff
 *
 */
typedef enum { kAccelerator, kBrake, kNumberOfPedals } Pedal;

/**
 * @brief   Initialize the pedals
 * @param   None
 * @return  None
 */
void PedalsInit(void);

/**
 * @brief   Provides the pedal distance pressed in percetage (accelerator or
 * brake)
 * @param   pedal_t pedal, ACCELERATOR or BRAKE as defined in enum
 * @return  distance the pedal has been pressed in percentage
 */
uint8_t PedalsRead(Pedal pedal);

#endif

/* @} */
