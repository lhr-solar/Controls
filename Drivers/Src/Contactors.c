/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file Contactors.h
 * @brief 
 * 
 */

#include "Contactors.h"
#include "stm32f4xx_gpio.h"
#include "Tasks.h"

static OS_MUTEX contactorsMutex;

/**
 * @brief   Helper function for setting contactors without mutex.
 *          Should only be called if mutex is held and struct contactor has been checked
 * @param   contactor the contactor
 *              (MOTOR_PRECHARGE/ARRAY_PRECHARGE/ARRAY_CONTACTOR)
 * @param   state the state to set (ON/OFF)
 * @return  None
 */ 
static void setContactor(contactor_t contactor, bool state) {
    switch (contactor) {
        case ARRAY_CONTACTOR :
            BSP_GPIO_Write_Pin(CONTACTORS_PORT, ARRAY_CONTACTOR_PIN, state);
            break;
        case ARRAY_PRECHARGE :
            BSP_GPIO_Write_Pin(CONTACTORS_PORT, ARRAY_PRECHARGE_PIN, state);
            break;
        case MOTOR_CONTACTOR :
            BSP_GPIO_Write_Pin(CONTACTORS_PORT, MOTOR_CONTACTOR_PIN, state);
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
void Contactors_Init() {
    BSP_GPIO_Init(CONTACTORS_PORT, 
                 (ARRAY_CONTACTOR_PIN) | 
                 (ARRAY_PRECHARGE_PIN) |
                 (MOTOR_CONTACTOR_PIN), 
                  1);

    // start disabled
    for (int contactor = 0; contactor < NUM_CONTACTORS; ++contactor) {
        setContactor(contactor, OFF);
    }

    // initialize mutex
    OS_ERR err;
    OSMutexCreate(&contactorsMutex, "Contactors lock", &err);
    assertOSError(err);
}

/**
 * @brief   Returns the current state of 
 *          a specified contactor
 * @param   contactor the contactor
 *              (MOTOR_PRECHARGE/ARRAY_PRECHARGE/ARRAY_CONTACTOR)
 * @return  The contactor's state (ON/OFF)
 */ 
bool Contactors_Get(contactor_t contactor) {
    State state = OFF;
    switch (contactor) {
        case ARRAY_CONTACTOR :
            state = BSP_GPIO_Get_State(CONTACTORS_PORT, ARRAY_CONTACTOR_PIN);
            break;
        case ARRAY_PRECHARGE :
            state = BSP_GPIO_Get_State(CONTACTORS_PORT, ARRAY_PRECHARGE_PIN);
            break;
        case MOTOR_CONTACTOR :
            state = BSP_GPIO_Get_State(CONTACTORS_PORT, MOTOR_CONTACTOR_PIN);
            break;
        default:
            break;
    }
    return (state == ON) ? ON : OFF;
}

/**
 * @brief   Sets the state of a specified contactor
 * @param   contactor the contactor
 *              (MOTOR_PRECHARGE/ARRAY_PRECHARGE/ARRAY_CONTACTOR)
 * @param   state the state to set (ON/OFF)
 * @param   blocking whether or not this should be a blocking call
 * @return  Whether or not the contactor was successfully set
 */
ErrorStatus Contactors_Set(contactor_t contactor, bool state, bool blocking) {
    CPU_TS timestamp;
    OS_ERR err;
    ErrorStatus result = ERROR;

    // acquire lock if its available
    OSMutexPend(&contactorsMutex, 0, blocking ? OS_OPT_PEND_BLOCKING : OS_OPT_PEND_NON_BLOCKING, &timestamp, &err);
    
    if(err == OS_ERR_PEND_WOULD_BLOCK){
        return ERROR;
    }
    assertOSError(err);

    // change contactor to match state and make sure it worked
    setContactor(contactor, state);
    bool ret = (bool)Contactors_Get(contactor);
    result = (ret == state) ? SUCCESS: ERROR;

    // release lock
    OSMutexPost(&contactorsMutex, OS_OPT_POST_NONE, &err);
    assertOSError(err);

    return result;
}
