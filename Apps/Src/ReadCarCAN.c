/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "ReadCarCAN.h"
#include "UpdateDisplay.h"
#include "Contactors.h"
#include "Minions.h"
#include "CAN_Queue.h"
#include "FaultState.h"
#include "os_cfg_app.h"



// CAN watchdog timer variables and constants
static OS_TMR canWatchTimer;
static const int CAN_WATCH_TMR_DLY_MS = 500;
static const int CAN_WATCH_TMR_DLY_TMR_TS = (CAN_WATCH_TMR_DLY_MS / DEF_TIME_NBR_mS_PER_SEC) * OS_CFG_TMR_TASK_RATE_HZ;

// prechargeDlyTimer and delay constant
static OS_TMR prechargeDlyTimer;
static const int PRECHARGE_DLY_TMR_TS = PRECHARGE_ARRAY_DELAY * OS_CFG_TMR_TASK_RATE_HZ;

// restart array and disable charging semaphores
static OS_SEM restartArraySem;
static OS_SEM canWatchDisableChargingSem;

// Array restart thread lock
static OS_MUTEX arrayRestartMutex;
static bool restartingArray = false;

// NOTE: This should not be written to anywhere other than ReadCarCAN. If the need arises, a mutex to protect it must be added.
// Indicates whether or not regenerative braking / charging is enabled.
static State regenEnable = OFF;

// Saturation buffer variables
static int chargeMsgBuffer[SAT_BUF_LENGTH];
static int saturationThreshold = 0;
static int chargeMsgSaturation;
static int oldestMsgIdx = 0;

// Handler to turn array back on
static void arrayRestart(void *p_tmr, void *p_arg); 

/**
 * @brief adds new messages by overwriting old messages in the saturation buffer and then updates saturation
 * @param chargeMessage whether bps message was charge enable (1) or disable (-1)
*/
static void updateSaturation(int chargeMessage){
    // Replace oldest message with new charge message and update index for oldest message
    chargeMsgBuffer[oldestMsgIdx] = chargeMessage;
    oldestMsgIdx = (oldestMsgIdx + 1) % SAT_BUF_LENGTH;

    // Calculate the new saturation value by assigning weightings from 1 to buffer length
    // in order of oldest to newest
    int newSaturation = 0;
    for (int i = 0; i < SAT_BUF_LENGTH; i++){
        newSaturation += chargeMsgBuffer[(oldestMsgIdx + i) % SAT_BUF_LENGTH] * (i + 1);
    }
    chargeMsgSaturation = newSaturation;
}

// helper function to call if charging should be disabled
static inline void chargingDisable(void) {
    // mark regen as disabled
    regenEnable = OFF;

    //kill contactors 
    Contactors_Set(ARRAY_CONTACTOR, OFF, SET_CONTACTORS_BLOCKING);
    Contactors_Set(ARRAY_PRECHARGE, OFF, SET_CONTACTORS_BLOCKING);
    
    // turn off the array contactor light
    UpdateDisplay_SetArray(false);

}

// helper function to call if charging should be enabled
static inline void chargingEnable(void) {
    OS_ERR err;
    CPU_TS ts;

    // mark regen as enabled
    regenEnable = ON;

    // check if we need to run the precharge sequence to turn on the array
    bool shouldRestartArray = false;

    OSMutexPend(&arrayRestartMutex,
                0,
                OS_OPT_PEND_BLOCKING,
                &ts,
                &err);
    assertOSError(OS_READ_CAN_LOC,err);

    // if nothing is precharging and we need to turn the array on, turn it on
    shouldRestartArray = !restartingArray && (Contactors_Get(ARRAY_CONTACTOR)==OFF) && (Contactors_Get(ARRAY_PRECHARGE)==OFF);

    // wait for precharge for array 
    if(shouldRestartArray){

            // Wait to make sure precharge is finished and then restart array
            OSTmrStart(&prechargeDlyTimer, &err);
            
        }
    

    OSMutexPost(&arrayRestartMutex,
                OS_OPT_NONE,
                &err);
    assertOSError(OS_READ_CAN_LOC,err);
}

/**
 * @brief Callback function for the precharge delay timer. Turns off precharge and restarts the array.
 * @param p_tmr pointer to the timer that calls this function, passed by timer
 * @param p_arg pointer to the argument passed by timer
 * 
*/
static void arrayRestart(void *p_tmr, void *p_arg){

    if(!regenEnable){    // If regen enable has been disabled during precharge, we don't want to turn on the main contactor immediately after
        restartingArray = false;
    }
    else {
       
            Contactors_Set(ARRAY_CONTACTOR, ON, SET_CONTACTORS_NON_BLOCKING);
    }

        //Lights_Set(A_CNCTR, ON); Don't know where this function is. Does it even exist?

        // done restarting the array
        restartingArray = false;

};

/**
 * @brief Disables charging when canWatchTimer hits 0 (when charge enable messages are missed)
 * @param p_tmr pointer to the timer that calls this function, passed by timer
 * @param p_arg pointer to the argument passed by timer
*/
void canWatchTimerCallback (void *p_tmr, void *p_arg){
    OS_ERR err;

    // mark regen as disabled
    regenEnable = OFF;
    
    // Set fault bitmaps 
    FaultBitmap |= FAULT_READBPS;
    OSErrLocBitmap |= OS_READ_CAN_LOC;

    // Signal fault state to kill contactors at its earliest convenience
    OSSemPost(&FaultState_Sem4, OS_OPT_POST_1, &err);
    assertOSError(OS_READ_CAN_LOC,err);

}



void Task_ReadCarCAN(void *p_arg)
{
    OS_ERR err;
    uint8_t buffer[8]; // buffer for CAN message
    uint32_t canId;

    OSMutexCreate(&arrayRestartMutex, "array restart mutex", &err);
    assertOSError(OS_READ_CAN_LOC,err);
    OSSemCreate(&restartArraySem, "array restart semaphore", 0, &err);
    assertOSError(OS_READ_CAN_LOC, err);
    OSSemCreate(&canWatchDisableChargingSem, "disable charging semaphore", 0, &err);

    // Calculate threshold value for the charge-enable saturation buffer based on buffer length
    // Values decrease by one each position, and the threshold is halfway between 0 and max
    for (int i = 1; i <= SAT_BUF_LENGTH; i++) {
        saturationThreshold += i;
    }
    saturationThreshold /= 2;


    // Create the CAN Watchdog (periodic) timer, which disconnects the array and disables regenerative braking
    // if we do not get a CAN message with the ID Charge_Enable within the desired interval.
    OSTmrCreate(
        &canWatchTimer,
        "CAN Watch Timer",
        0,
        CAN_WATCH_TMR_DLY_TMR_TS,
        OS_OPT_TMR_PERIODIC,
        canWatchTimerCallback,
        NULL,
        &err
    );
    assertOSError(OS_READ_CAN_LOC, err);

    OSTmrCreate(
        &prechargeDlyTimer,
        "Precharge Delay Timer",
        PRECHARGE_DLY_TMR_TS,
        0,
        OS_OPT_TMR_ONE_SHOT,
        arrayRestart,
        NULL,
        &err
    );
    assertOSError(OS_READ_CAN_LOC, err);

    //Start CAN Watchdog timer
    OSTmrStart(&canWatchTimer, &err);
    

    while (1)
    {

        
        //Get any message that BPS Sent us
        ErrorStatus status = CANbus_Read(&canId, buffer, CAN_BLOCKING); 
        if(status != SUCCESS) {
            continue;
        }

        switch(canId){ //we got a message
            case CHARGE_ENABLE: { 

                // Restart CAN Watchdog timer
                OSTmrStart(&canWatchTimer, &err);

                if(buffer[0] == 0){ // If the buffer doesn't contain 1 for enable, turn off RegenEnable and turn array off
                    chargingDisable();
                    updateSaturation(-1); // Update saturation and buffer

                } else { //We got a message of enable with a nonzero value
                    updateSaturation(1);

                    // If the charge message saturation is above the threshold, wait for precharge and restart sequence then enable charging.
                    // If we are already precharging/array is on, nothing will be done 
                    if (chargeMsgSaturation >= saturationThreshold){
                        chargingEnable();
                    }
                    
                }
                break;
            }
            case SUPPLEMENTAL_VOLTAGE: {
                UpdateDisplay_SetSBPV(*(uint16_t *) &buffer); // Receive value in mV
                break;
            }
            case STATE_OF_CHARGE:{
                uint8_t SOC = (*(uint32_t*) &buffer)/(100000);  // Convert to integer percent
                UpdateDisplay_SetSOC(SOC);
                break;
            }
            default: 
                break;
        }
    }
}


bool RegenEnable_Get() 
{
    return regenEnable;
}




