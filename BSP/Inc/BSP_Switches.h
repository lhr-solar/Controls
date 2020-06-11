/**
 * Header file for the library to interact
 * with the switches on the steering wheel
 */

#ifndef __BSP_SWITCHES_H
#define __BSP_SWITCHES_H

#include "common.h"

typedef enum {LT, RT, HDLT, FWD_REV, HZD, CRS_SET, CRS_EN, REGEN} switch_t;

/**
 * @brief   Initializes all switches
 *          from the steering wheel
 * @param   None
 * @return  None
 */ 
void BSP_Switches_Init(void);

/**
 * @brief   Reads the current state of 
 *          the specified switch
 * @param   None
 * @return  None
 */ 
void BSP_Switches_Read(switch_t sw);

#endif
