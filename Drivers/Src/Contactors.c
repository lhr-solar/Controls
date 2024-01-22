/**
 * @file Contactors.c
 * @details
 * The contactors can be set using a blocking or non-blocking interface. 
 * The blocking interface will wait until the contactor can be set, while 
 * the non-blocking interface will return early if another thread is currently 
 * setting the contactor.
 * 
 * The contactors are controlled via GPIO writes to the contactors board.
 * 
 */

#include "Contactors.h"

#include "Tasks.h"

static OS_MUTEX contactors_mutex;

/**
 * @brief   Helper function for setting contactors without mutex.
 *          Should only be called if mutex is held and struct contactor has been
 * checked
 * @param   contactor the contactor
 *              (kMotorControllerPrechargeBypassContactor/kArrayPrechargeBypassContactor)
 * @param   state the state to set (ON/OFF)
 * @return  None
 */
static void setContactor(Contactor contactor, bool state) {
    switch (contactor) {
        case kArrayPrechargeBypassContactor:
            BspGpioWritePin(CONTACTORS_PORT, ARRAY_PRECHARGE_BYPASS_PIN, state);
            break;
        case kMotorControllerPrechargeBypassContactor:
            BspGpioWritePin(CONTACTORS_PORT,
                            MOTOR_CONTROLLER_PRECHARGE_BYPASS_PIN, state);
            break;
        default:
            break;
    }
}

/**
 * @brief   Initializes contactors to be used
 *          in connection with the Motor and Array
 * @return  None
 */
void ContactorsInit() {
    BspGpioInit(
        CONTACTORS_PORT,
        (ARRAY_PRECHARGE_BYPASS_PIN) | (MOTOR_CONTROLLER_PRECHARGE_BYPASS_PIN),
        1);

    // start disabled
    for (int contactor = 0; contactor < kNumContactors; ++contactor) {
        setContactor(contactor, OFF);
    }

    // initialize mutex
    OS_ERR err = 0;
    OSMutexCreate(&contactors_mutex, "Contactors lock", &err);
    ASSERT_OS_ERROR(err);
}

/**
 * @brief   Returns the current state of
 *          a specified contactor
 * @param   contactor MOTOR_CONTROLLER_PRECHARGE_BYPASS_CONTACTOR or ARRAY_PRECHARGE_BYPASS_CONTACTOR
 * @return  Contactor state (ON/OFF)
 */ 
bool ContactorsGet(Contactor contactor) {
    State state = OFF;
    switch (contactor) {
        case kArrayPrechargeBypassContactor:
            state =
                BspGpioGetState(CONTACTORS_PORT, ARRAY_PRECHARGE_BYPASS_PIN);
            break;
        case kMotorControllerPrechargeBypassContactor:
            state = BspGpioGetState(CONTACTORS_PORT,
                                    MOTOR_CONTROLLER_PRECHARGE_BYPASS_PIN);
            break;
        default:
            break;
    }
    return (state == ON) ? ON : OFF;
}

/**
 * @brief   Sets the state of a specified contactor
 * @param   contactor the contactor
 *              (kMotorControllerPrechargeBypassContactor/kArrayPrechargeBypassContactor)
 * @param   state the state to set (ON/OFF)
 * @param   blocking whether or not this should be a blocking call
 * @return  Whether or not the contactor was successfully set
 */
ErrorStatus ContactorsSet(Contactor contactor, bool state, bool blocking) {
    CPU_TS timestamp = 0;
    OS_ERR err = 0;
    ErrorStatus result = ERROR;

    // acquire lock if its available
    OSMutexPend(&contactors_mutex, 0,
                blocking ? OS_OPT_PEND_BLOCKING : OS_OPT_PEND_NON_BLOCKING,
                &timestamp, &err);

    if (err == OS_ERR_PEND_WOULD_BLOCK) {
        return ERROR;
    }
    ASSERT_OS_ERROR(err);

    // change contactor to match state and make sure it worked
    setContactor(contactor, state);
    bool ret = (bool)ContactorsGet(contactor);
    result = (ret == state) ? SUCCESS : ERROR;

    // release lock
    OSMutexPost(&contactors_mutex, OS_OPT_POST_NONE, &err);
    ASSERT_OS_ERROR(err);

    return result;
}
