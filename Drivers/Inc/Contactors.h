/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file Contactors.h
 * @brief The contactor driver is responsible for communication 
 * with the array and motor controller precharge bypass contactors. It
 * provides a simple interface to set and get the state of the contactors.
 * 
 */

#ifndef __CONTACTORS_H
#define __CONTACTORS_H

#include "common.h"
#include "config.h"
#include "BSP_GPIO.h"
#include "stm32f4xx_gpio.h"

/**
 * @brief  The GPIO port used for the contactors
 */
#define CONTACTORS_PORT                                 PORTC

/**
 * @brief  The GPIO pin used for the array precharge bypass contactor
 */
#define ARRAY_PRECHARGE_BYPASS_PIN                      GPIO_Pin_10

/**
 * @brief  The GPIO pin used for the motor controller precharge bypass contactor
 */
#define MOTOR_CONTROLLER_PRECHARGE_BYPASS_PIN           GPIO_Pin_12     

#define FOREACH_contactor(contactor) \
        contactor(ARRAY_PRECHARGE_BYPASS_CONTACTOR),  \
        contactor(MOTOR_CONTROLLER_PRECHARGE_BYPASS_CONTACTOR),   \

/**
 * @brief The contactors that can be set
*/
typedef enum {
    FOREACH_contactor(GENERATE_ENUM)
    NUM_CONTACTORS,
} contactor_t;

/**
 * @brief   Initializes contactors to be used
 *          in connection with the Motor and Array
 */  
void Contactors_Init(void);

/**
 * @brief   Returns the current state of 
 *          a specified contactor
 * @param   contactor MOTOR_CONTROLLER_PRECHARGE_BYPASS_CONTACTOR or ARRAY_PRECHARGE_BYPASS_CONTACTOR
 * @return  Contactor state (ON/OFF)
 */ 
bool Contactors_Get(contactor_t contactor);

/**
 * @brief   Sets the state of a specified contactor
 * @param   contactor MOTOR_CONTROLLER_PRECHARGE_BYPASS_CONTACTOR or ARRAY_PRECHARGE_BYPASS_CONTACTOR
 * @param   state the state to set (ON/OFF)
 * @param   blocking whether or not this should be a blocking call
 * @return  Whether or not the contactor was successfully set
 */
ErrorStatus Contactors_Set(contactor_t contactor, bool state, bool blocking);

#endif



