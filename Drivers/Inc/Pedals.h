/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file Pedals.h
 * @brief Header file for the Pedals driver
 * 
 */

#ifndef __PEDALS_H
#define __PEDALS_H

#include "BSP_ADC.h"

/**
 * @brief Stuff
 * 
 */
typedef enum 
{
    ACCELERATOR, 
    BRAKE,
    NUMBER_OF_PEDALS
} pedal_t;

/**
 * @brief   Initializes the brake and accelerator by using the 
 *          BSP_ADC_Init function
 * @return  None
 */
void Pedals_Init(void);

/**
 * @brief   Fetches the millivoltage value of the potentiomenter as provided 
 *          by the ADC channel of the requested pedal (Accelerator or Brake),
 *          converts it to a percentage of the total distance pressed using 
 *          data from calibration testing, and returns it
 * @param   pedal_t ACCELERATOR or BRAKE
 * @return  percent amount the pedal has been pressed in percentage
 */
int8_t Pedals_Read(pedal_t pedal);


#endif



