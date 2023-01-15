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

static const char *CONTACTOR_STRING[] = {
    FOREACH_contactor(GENERATE_STRING)
};


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
State Contactors_Get(contactor_t contactor);

/**
 * @brief   Sets the state of a specified contactor
 * @param   contactor the contactor (MOTOR/ARRAY)
 * @return  Whether or not the contactor was successfully set
 */
ErrorStatus Contactors_Set(contactor_t contactor, State state);

/**
 * @brief   Enable the specified contactor
 *          Contactor will turn on if enabled and state is on
 * @return  None
 */ 
void Contactors_Enable(contactor_t contactor);

/**
 * @brief   Disable the specified contactor
 * @return  None
 */ 
void Contactors_Disable(contactor_t contactor);

#endif