/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file Contactors.h
 * @brief
 *
 * @defgroup Contactors
 * @addtogroup Contactors
 * @{
 */

#ifndef CONTACTORS_H
#define CONTACTORS_H

#include "BSP_GPIO.h"
#include "common.h"
#include "config.h"

#define CONTACTORS_PORT kPortC
#define ARRAY_PRECHARGE_BYPASS_PIN GPIO_Pin_10
#define MOTOR_CONTROLLER_PRECHARGE_BYPASS_PIN GPIO_Pin_12

#define FOREACH_CONTACTOR(contactor)           \
    contactor(kArrayPrechargeBypassContactor), \
        contactor(kMotorControllerPrechargeBypassContactor),

typedef enum ContactorEnum {
    FOREACH_CONTACTOR(GENERATE_ENUM) kNumContactors,
} Contactor;

/**
 * @brief   Initializes contactors to be used
 *          in connection with the Motor and Array
 * @return  None
 */
void ContactorsInit();

/**
 * @brief   Returns the current state of
 *          a specified contactor
 * @param   contactor the contactor
 *              (kMotorControllerPrechargeBypassContactor/kArrayPrechargeBypassContactor)
 * @return  The contactor's state (ON/OFF)
 */
bool ContactorsGet(Contactor contactor);

/**
 * @brief   Sets the state of a specified contactor
 * @param   contactor the contactor
 * (kMotorControllerPrechargeBypassContactor/kArrayPrechargeBypassContactor)
 * @param   state the state to set (ON/OFF) (true/false)
 * @param   blocking whether or not this should be a blocking call
 * @return  Whether or not the contactor was successfully set
 */
ErrorStatus ContactorsSet(Contactor contactor, bool state, bool blocking);

#endif

/* @} */
