/* Copyright (c) 2020 UT Longhorn Racing Solar */

/**
 * Header file for the Pedals driver
 */

/** @defgroup Pedals */
/** @ingroup Pedals 
 * @{ */

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
int8_t Pedals_Read(pedal_t pedal);


#endif

/*@}*/
