/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file Contactors.h
 * @brief 
 * 
 */

#include "Contactors.h"
#include "stm32f4xx_gpio.h"
#include "Tasks.h"
#include "BSP_GPIO.h"
#include "daybreak_pins.h"


static OS_MUTEX contactorsMutex;

// Array describes the state of the Contactors. 
static bool contactorState[NUM_CONTACTORS];

/**
 * @brief   Helper function for setting contactors without mutex.
 *          Should only be called if mutex is held and struct contactor has been checked
 * @param   contactor the contactor
 *              (MOTOR_CONTROLLER_PRECHARGE_BYPASS_CONTACTOR/ARRAY_PRECHARGE_BYPASS_CONTACTOR)
 * @param   state the state to set (ON/OFF)
 * @return  None
 */ 
static void setContactor(contactor_t contactor, bool state) {
    switch (contactor) {
        case MOTOR_CONTROLLER_CONTACTOR:
            contactorState[MOTOR_CONTROLLER_CONTACTOR] = state;
            BSP_GPIO_Write_Pin(MOTOR_CONTACTOR_PORT, MOTOR_CONTACTOR, state);
            break;

        // Precharge Contactors are updated by ReadCarCAN.c
        case MOTOR_CONTROLLER_PRECHARGE_BYPASS_CONTACTOR:
            contactorState[MOTOR_CONTROLLER_PRECHARGE_BYPASS_CONTACTOR] = state;
            break;
        case ARRAY_PRECHARGE_BYPASS_CONTACTOR:
            contactorState[ARRAY_PRECHARGE_BYPASS_CONTACTOR] = state;
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
    // Motor Contactor pins
    BSP_GPIO_Init(MOTOR_CONTACTOR_PORT, MOTOR_CONTACTOR, OUTPUT, false); // control
    BSP_GPIO_Init_PullUp(MOTOR_C_SENSE_PORT, MOTOR_C_SENSE, INPUT, true); // sense



    // start disabled
    for (int contactor = 0; contactor < NUM_CONTACTORS; ++contactor) {
        // Only Motor Contactor is directly controlled by Controls
        setContactor(contactor, OFF);
        contactorState[contactor] = OFF;
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
 *              (MOTOR_CONTROLLER_PRECHARGE_BYPASS_CONTACTOR/ARRAY_PRECHARGE_BYPASS_CONTACTOR)
 * @return  The contactor's state (ON/OFF)
 */ 
bool Contactors_Get(contactor_t contactor) {
    switch (contactor) {
        case MOTOR_CONTROLLER_CONTACTOR:
            contactorState[MOTOR_CONTROLLER_CONTACTOR] = BSP_GPIO_Get_State(MOTOR_CONTACTOR_PORT, MOTOR_CONTACTOR) == 0 ? ON : OFF;
            break;
        // Precharge Contactors are updated by ReadCarCAN.c
        case ARRAY_PRECHARGE_BYPASS_CONTACTOR :
            break;
        case MOTOR_CONTROLLER_PRECHARGE_BYPASS_CONTACTOR :
            break;
        default:
            break;
    }
    return contactorState[contactor];
}

/**
 * @brief   Sets the state of a specified contactor
 * @param   contactor the contactor
 *              (MOTOR_CONTROLLER_PRECHARGE_BYPASS_CONTACTOR/ARRAY_PRECHARGE_BYPASS_CONTACTOR)
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
    // TODO: add delay between sense reads
    bool ret = Contactors_Get(contactor);
    result = (ret == state) ? SUCCESS: ERROR;

    // release lock
    OSMutexPost(&contactorsMutex, OS_OPT_POST_NONE, &err);
    assertOSError(err);

    return result;
}

void Contactors_DisableAll(){
    for(uint8_t i = 0; i < NUM_CONTACTORS; i++){
        Contactors_Set(i, true, OFF);
    }
}

/**
 * @brief   Disables all contactors and bypasses mutex
 *          Should I only used in a fault state
 *          Note: NOT not turn off Contactors not controlled by Controls
 * @return  None
 */
void Contactors_EmergencyDisable() {
    for (uint8_t i = 0; i < NUM_CONTACTORS; i++) {
        setContactor(i, OFF);
    }
}
