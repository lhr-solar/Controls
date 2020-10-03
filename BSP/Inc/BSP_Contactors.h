/* Copyright (c) 2020 UT Longhorn Racing Solar */

#ifndef __BSP_CONTACTORS_H
#define __BSP_CONTACTORS_H

#include "common.h"
#include "config.h"
#include <bsp.h>

typedef enum {MOTOR=0, ARRAY, NUM_CONTACTORS} contactor_t;
/**
 * @brief   Initializes contactors to be used
 *          in connection with the Motor and Array
 * @param   contactor the contactor to initialize
 * @return  None
 */ 
void BSP_Contactors_Init(contactor_t contactor);

/**
 * @brief   Reads contactor states from a file 
 *          and returns the current state of 
 *          a specified contactor
 * @param   contactor specifies the contactor for 
 *          which the user would like to know its state (MOTOR/ARRAY)
 * @return  The contactor's state (ON/OFF)
 */ 
State BSP_Contactors_Get(contactor_t contactor);

/**
 * @brief   Sets the state of a specified contactor
 *          by updating csv file
 * @param   contactor specifies the contactor for 
 *          which the user would like to set its state (MOTOR/ARRAY)
 * @param   state specifies the state that 
 *          the user would like to set (ON/OFF)
 * @return  The contactor's state (ON/OFF)
 */ 
void BSP_Contactors_Set(contactor_t contactor, State state);

#endif
