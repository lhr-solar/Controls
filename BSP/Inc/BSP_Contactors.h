#ifndef __BSP_CONTACTORS_H
#define __BSP_CONTACTORS_H

#include "common.h"
#include "config.h"
#include <bsp.h>

typedef enum {MOTOR=0, ARRAY} ContactorType_t;
/**
 * @brief   Confirms that the CSV file
 *          has been created and creates
 *          one if not
 * @param   None
 * @return  None
 */ 
void BSP_Contactors_Init(void);

/**
 * @brief   Reads contactor states from a file 
 *          and returns the current state of 
 *          a specified contactor
 * @param   contactor specifies the contactor for 
 *          which the user would like to know its state (MOTOR/ARRAY)
 * @return  The contactor's state (ON/OFF)
 */ 
state_t BSP_Contactors_Get(ContactorType_t contactor);

/**
 * @brief   Sets the state of a specified contactor
 *          by updating csv file
 * @param   contactor specifies the contactor for 
 *          which the user would like to set its state (MOTOR/ARRAY)
 * @param   state specifies the state that 
 *          the user would like to set (ON/OFF)
 * @return  The contactor's state (ON/OFF)
 */ 
void BSP_Contactors_Set(ContactorType_t contactor, state_t state);

#endif