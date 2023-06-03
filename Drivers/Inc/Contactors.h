/* Copyright (c) 2020 UT Longhorn Racing Solar */

#ifndef __CONTACTORS_H
#define __CONTACTORS_H

#include "common.h"
#include "config.h"
#include "BSP_GPIO.h"
#include "stm32f4xx_gpio.h"

#define CONTACTORS_PORT         PORTC
#define ARRAY_PRECHARGE_PIN GPIO_Pin_10
#define ARRAY_CONTACTOR_PIN GPIO_Pin_11
#define MOTOR_CONTACTOR_PIN GPIO_Pin_12

#define FOREACH_contactor(contactor) \
        contactor(ARRAY_CONTACTOR)   \
        contactor(ARRAY_PRECHARGE)  \
        contactor(MOTOR_CONTACTOR)   \

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

typedef enum contactor_ENUM {
    FOREACH_contactor(GENERATE_ENUM)
    CONTACTOR_NUM,
}contactor_t;



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
 *              (MOTOR_PRECHARGE/ARRAY_PRECHARGE/ARRAY_CONTACTOR)
 * @return  The contactor's state (ON/OFF)
 */ 
bool Contactors_Get(contactor_t contactor);

/**
 * @brief   Sets the state of a specified contactor
 * @param   contactor the contactor (MOTOR/ARRAY)
 * @param   state the state to set (ON/OFF) (true/false)
 * @param   blocking whether or not this should be a blocking call
 * @return  Whether or not the contactor was successfully set
 */
ErrorStatus Contactors_Set(contactor_t contactor, bool state, bool blocking);

#endif