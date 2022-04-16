/* Copyright (c) 2020 UT Longhorn Racing Solar */


#include "Contactors.h"
#include "stm32f4xx_gpio.h"
#include "Tasks.h"

#define CONTACTORS_PORT         PORTC
#define ARRAY_PRECHARGE_PIN GPIO_Pin_10
#define ARRAY_CONTACTOR_PIN GPIO_Pin_11
#define MOTOR_CONTACTOR_PIN GPIO_Pin_12

// keep track of contactor desired state and if enabled here
struct contactor {
    State state;
    bool enabled;
};
static struct contactor contactors[NUM_CONTACTORS];
static OS_MUTEX contactorsMutex;

/**
 * @brief   Helper function for setting contactors without mutex.
 *          Should only be called if mutex is held and struct contactor has been checked
 * @param   contactor the contactor
 *              (MOTOR_PRECHARGE/ARRAY_PRECHARGE/ARRAY_CONTACTOR)
 * @param   state the state to set (ON/OFF)
 * @return  None
 */ 
static void setContactor(contactor_t contactor, State state) {
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

    // start contactors off and disabled
    for (int contactor = 0; contactor < NUM_CONTACTORS; ++contactor) {
        contactors[contactor].state = OFF;
        contactors[contactor].enabled = false;
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
 * @return  The contactor's state (ON/OFF)
 */ 
State Contactors_Get(contactor_t contactor) {
    uint8_t state = 0;
    switch (contactor) {
        case ARRAY_CONTACTOR :
            state = BSP_GPIO_Read_Pin(CONTACTORS_PORT, ARRAY_CONTACTOR_PIN);
            break;
        case ARRAY_PRECHARGE :
            state = BSP_GPIO_Read_Pin(CONTACTORS_PORT, ARRAY_PRECHARGE_PIN);
            break;
        case MOTOR_CONTACTOR :
            state = BSP_GPIO_Read_Pin(CONTACTORS_PORT, MOTOR_CONTACTOR_PIN);
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
 * @return  None
 */
void Contactors_Set(contactor_t contactor, State state) {
    CPU_TS timestamp;
    OS_ERR err;

    // acquire lock
    OSMutexPend(&contactorsMutex, 0, OS_OPT_PEND_BLOCKING, &timestamp, &err);
    assertOSError(OS_CONTACTOR_LOC, err);

    // update contactor state
    contactors[contactor].state = state;

    // if enabled, change contactor to match state
    if (contactors[contactor].enabled) {
        setContactor(contactor, state);
    }

    // release lock
    OSMutexPost(&contactorsMutex, OS_OPT_POST_NONE, &err);
    assertOSError(OS_CONTACTOR_LOC, err);
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

    // if desired state is ON, turn contactor ON
    if (contactors[contactor].state == ON) {
        setContactor(contactor, ON);
    }

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

    // mark contactor as disabled and off
    contactors[contactor].enabled = false;
    contactors[contactor].state = OFF;

    // turn off the contactor
    Contactors_Set(contactor, OFF);

    // release lock
    OSMutexPend(&contactorsMutex, 0, OS_OPT_PEND_BLOCKING, &timestamp, &err);
    assertOSError(OS_CONTACTOR_LOC, err);

}