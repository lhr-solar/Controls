/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file BSP_PWM.h
 * @brief Header file for the library to interact
 * with the Timer (PWM)
 * 
 * @defgroup BSP_PWM
 * @addtogroup BSP_PWM
 * @{
 */

#ifndef __BSP_PWM_H
#define __BSP_PWM_H

#include "bsp.h"
#include "common.h"
#include "config.h"
#include "daybreak_pins.h"


/**
 * @brief   Initialize the ADC module
 * @return  None
 */ 
void BSP_PWM_Init(void);


#endif
