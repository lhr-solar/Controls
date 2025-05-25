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

#define CONTACTOR_SENSE_DELAY 1


// Masks for Contactor driver board's can message
#define ARRAY_PRECHARGE_SENSE_FAULT(dataBuf)   (bool)((dataBuf[1] >> 0) & 0x01) // Bit 8: Array precharge sense fault happened
#define ARRAY_PRECHARGE_EXPECTED_VALUE(dataBuf) (bool)((dataBuf[0] >> 7) & 0x01) // Bit 7: Expected array precharge sense value
#define ARRAY_PRECHARGE_ACTUAL_VALUE(dataBuf)   (bool)((dataBuf[0] >> 6) & 0x01) // Bit 6: Actual array precharge sense value

#define MOTOR_PRECHARGE_SENSE_FAULT(dataBuf)   (bool)((dataBuf[0] >> 5) & 0x01) // Bit 5: Motor precharge sense fault happened
#define MOTOR_PRECHARGE_EXPECTED_VALUE(dataBuf) (bool)((dataBuf[0] >> 4) & 0x01) // Bit 4: Expected motor precharge sense value
#define MOTOR_PRECHARGE_ACTUAL_VALUE(dataBuf)   ((dataBuf[0] >> 3) & 0x01) // Bit 3: Actual motor precharge sense value

#define MOTOR_SENSE_FAULT(dataBuf)             (bool)((dataBuf[0] >> 2) & 0x01) // Bit 2: Motor sense fault happened
#define MOTOR_SENSE_EXPECTED_VALUE(dataBuf)    (bool)((dataBuf[0] >> 1) & 0x01) // Bit 1: Expected motor sense value
#define MOTOR_SENSE_ACTUAL_VALUE(dataBuf)      (bool)((dataBuf[0] >> 0) & 0x01) // Bit 0: Actual motor sense value
 

#define FOREACH_contactor(contactor)             \
    contactor(ARRAY_PRECHARGE_BYPASS_CONTACTOR), \
    contactor(MOTOR_CONTROLLER_CONTACTOR), \
    contactor(MOTOR_CONTROLLER_PRECHARGE_BYPASS_CONTACTOR), \
    contactor(ARRAY_CONTACTOR), \
    contactor(HV_PLUS_CONTACTOR), \
    contactor(HV_MINUS_CONTACTOR),

typedef enum contactor_ENUM
{
    FOREACH_contactor(GENERATE_ENUM)
        NUM_CONTACTORS
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
 * @param   blocking whether or not this should be a blocking call
 * @return  The contactor's state (ON/OFF)
 */
bool Contactors_Get(contactor_t contactor, bool blocking);

/**
 * @brief   Sets the state of a specified contactor
 * @param   contactor the contactor (MOTOR_CONTROLLER_PRECHARGE_BYPASS_CONTACTOR/ARRAY_PRECHARGE_BYPASS_CONTACTOR)
 * @param   state the state to set (ON/OFF) (true/false)
 * @param   blocking whether or not this should be a blocking call
 * @return  Whether or not the contactor was successfully set
 */
ErrorStatus Contactors_Set(contactor_t contactor, bool state, bool blocking);

/**
 * @brief   Disables all contactors and bypasses mutex
 *          Should I only used in a fault state
 *          Note: NOT not turn off Contactors not controlled by Controls
 * @return  None
 */
void MotorContactor_EmergencyDisable(void);

/**
 * @brief   Disables motor contactor and bypasses mutex
 *          Should only used in a fault state
 * @param   None
 * @return  None
 */
void Contactors_DisableAll(void);

#endif

/* @} */
