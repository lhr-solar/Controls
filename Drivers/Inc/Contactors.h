/**
 * @file Contactors.h
 * @brief The contactor driver is responsible for communication
 * with the array and motor controller precharge bypass contactors. It
 * provides a simple interface to set and get the state of the contactors.
 *
 */

#ifndef CONTACTORS_H
#define CONTACTORS_H

#include "BSP_GPIO.h"
#include "common.h"
#include "config.h"
#include "stm32f4xx_gpio.h"

/**
 * @brief  The GPIO port used for the contactors
 */
#define CONTACTORS_PORT kPortC

/**
 * @brief  The GPIO pin used for the array precharge bypass contactor
 */
#define ARRAY_PRECHARGE_BYPASS_PIN GPIO_Pin_10

/**
 * @brief  The GPIO pin used for the motor controller precharge bypass contactor
 */
#define MOTOR_CONTROLLER_PRECHARGE_BYPASS_PIN GPIO_Pin_12

#define FOREACH_CONTACTOR(contactor)           \
    contactor(kArrayPrechargeBypassContactor), \
        contactor(kMotorControllerPrechargeBypassContactor),

/**
 * @brief The contactors that can be set
 */
typedef enum ContactorEnum {
    FOREACH_CONTACTOR(GENERATE_ENUM) kNumContactors,
} Contactor;

/**
 * @brief   Initializes contactors to be used
 *          in connection with the Motor and Array
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
