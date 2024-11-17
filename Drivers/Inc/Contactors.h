/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file Contactors.h
 * @brief
 *
 * @defgroup Contactors
 * @addtogroup Contactors
 * @{
 */

#ifndef __CONTACTORS_H
#define __CONTACTORS_H

#include "common.h"
#include "config.h"
#include "BSP_GPIO.h"
#include "stm32f4xx_gpio.h"

#define CONTACTORS_PORT PORTC
#define ARRAY_PRECHARGE_BYPASS_PIN                 GPIO_Pin_11
#define MOTOR_CONTROLLER_PRECHARGE_BYPASS_PIN    GPIO_Pin_10

#define FOREACH_contactor(contactor)             \
    contactor(ARRAY_PRECHARGE_BYPASS_CONTACTOR), \
        contactor(MOTOR_CONTROLLER_PRECHARGE_BYPASS_CONTACTOR),

typedef enum contactor_ENUM
{
    FOREACH_contactor(GENERATE_ENUM)
        NUM_CONTACTORS,
} contactor_t;

/**
 * @brief   Initializes contactors to be used
 *          in connection with the Motor and Array
 * @return  None
 */
void Contactors_Init();

/**
 * @brief   Returns the current state of
 *          a specified contactor
 * @param   contactor the contactor
 *              (MOTOR_CONTROLLER_PRECHARGE_BYPASS_CONTACTOR/ARRAY_PRECHARGE_BYPASS_CONTACTOR)
 * @return  The contactor's state (ON/OFF)
 */
bool Contactors_Get(contactor_t contactor);

/**
 * @brief   Sets the state of a specified contactor
 * @param   contactor the contactor (MOTOR_CONTROLLER_PRECHARGE_BYPASS_CONTACTOR/ARRAY_PRECHARGE_BYPASS_CONTACTOR)
 * @param   state the state to set (ON/OFF) (true/false)
 * @param   blocking whether or not this should be a blocking call
 * @return  Whether or not the contactor was successfully set
 */
ErrorStatus Contactors_Set(contactor_t contactor, bool state, bool blocking);

#endif

/* @} */
