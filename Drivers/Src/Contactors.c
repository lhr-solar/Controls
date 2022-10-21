/* Copyright (c) 2020 UT Longhorn Racing Solar */


#include "Contactors.h"
#include "stm32f4xx_gpio.h"
#include "Tasks.h"

// keep track of contactor desired state and if enabled here
struct contactor {
    bool enabled;
};
static struct contactor contactors[NUM_CONTACTORS];
static OS_MUTEX contactorsMutex;

/**
 * @brief   Helper function for setting contactors without mutex.
 *          Should only be called if mutex is held and struct contactor has been checked
 * @param   contactor the contactor
 *              (MOTOR_PRECHARGE/ARRAY_PRECHARGE/ARRAY_CONTACTOR)
 * @param   state the state to set (true/false)
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
        contactors[contactor].enabled = false;
        setContactor(contactor, false);
    }

    // initialize mutex
    OS_ERR err;
    OSMutexCreate(&contactorsMutex, "Contactors lock", &err);
    assertOSError(OS_CONTACTOR_LOC, err);
}

/**
 * @brief   Returns the current state of 
 *          a specified contactor
 * @param   contactor the contactor
 *              (MOTOR_PRECHARGE/ARRAY_PRECHARGE/ARRAY_CONTACTOR)
 * @return  The contactor's state (true/false)
 */ 
bool Contactors_Get(contactor_t contactor) {
    bool state = false;
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
    return (state == true) ? true : false;
}

/**
 * @brief   Sets the state of a specified contactor
 * @param   contactor the contactor
 *              (MOTOR_PRECHARGE/ARRAY_PRECHARGE/ARRAY_CONTACTOR)
 * @param   state the state to set (true/false)
 * @return  Whether or not the contactor was successfully set
 */
ErrorStatus Contactors_Set(contactor_t contactor, bool state) {
    CPU_TS timestamp;
    OS_ERR err;
    ErrorStatus result = ERROR;

    // acquire lock
    OSMutexPend(&contactorsMutex, 0, OS_OPT_PEND_BLOCKING, &timestamp, &err);
    assertOSError(OS_CONTACTOR_LOC, err);

    // if enabled, change contactor to match state
    if (contactors[contactor].enabled) {
        setContactor(contactor, state);
        result = SUCCESS;
    }

    // release lock
    OSMutexPost(&contactorsMutex, OS_OPT_POST_NONE, &err);
    assertOSError(OS_CONTACTOR_LOC, err);

    return result;
}

/**
 * @brief   Enable the specified contactor
 *          Contactor will turn on if enabled and state is on
 * @return  None
 */ 
void Contactors_Enable(contactor_t contactor) {
    CPU_TS timestamp;
    OS_ERR err;

    // acquire lock
    OSMutexPend(&contactorsMutex, 0, OS_OPT_PEND_BLOCKING, &timestamp, &err);
    assertOSError(OS_CONTACTOR_LOC, err);

    // mark contactor as enabled
    contactors[contactor].enabled = true;

    // release lock
    OSMutexPost(&contactorsMutex, OS_OPT_POST_NONE, &err);
    assertOSError(OS_CONTACTOR_LOC, err);
}

/**
 * @brief   Disable the specified contactor
 * @return  None
 */ 
void Contactors_Disable(contactor_t contactor) {
    CPU_TS timestamp;
    OS_ERR err;

    // acquire lock
    OSMutexPend(&contactorsMutex, 0, OS_OPT_PEND_BLOCKING, &timestamp, &err);
    assertOSError(OS_CONTACTOR_LOC, err);

    // mark contactor as disabled
    contactors[contactor].enabled = false;

    // turn off the contactor
    setContactor(contactor, false);

    // release lock
    OSMutexPost(&contactorsMutex, OS_OPT_POST_NONE, &err);
    assertOSError(OS_CONTACTOR_LOC, err);

}
