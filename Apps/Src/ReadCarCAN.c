/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "ReadCarCAN.h"
#include "UpdateDisplay.h"
#include "Contactors.h"
#include "Minions.h"
#include "CAN_Queue.h"
#include "FaultState.h"
#include "os_cfg_app.h"

// Saturation threshold is halfway between 0 and max saturation value (half of summation from one to the number of positions)
#define saturationThreshold (((SAT_BUF_LENGTH + 1) * SAT_BUF_LENGTH) / 4) 

// timer delay constants
#define CAN_WATCH_TMR_DLY_MS 500u
#define CAN_WATCH_TMR_DLY_TMR_TS ((CAN_WATCH_TMR_DLY_MS * OS_CFG_TMR_TASK_RATE_HZ) / (1000u)) //1000 for ms -> s conversion
#define PRECHARGE_DLY_TMR_TS (PRECHARGE_ARRAY_DELAY * OS_CFG_TMR_TASK_RATE_HZ)

// CAN watchdog timer variable
static OS_TMR canWatchTimer;

// prechargeDlyTimer and delay constant
static OS_TMR prechargeDlyTimer;

// Array restart thread lock
static OS_MUTEX arrayRestartMutex;
static bool restartingArray = false;

// NOTE: This should not be written to anywhere other than ReadCarCAN. If the need arises, a mutex to protect it must be added.
// Indicates whether or not regenerative braking / charging is enabled.
static bool regenEnable = false;

// Saturation buffer variables
static int8_t chargeMsgBuffer[SAT_BUF_LENGTH];
static int chargeMsgSaturation = 0;
static uint8_t oldestMsgIdx = 0;

// Handler to turn array back on
static void arrayRestart(void *p_tmr, void *p_arg); 

/**
 * @brief adds new messages by overwriting old messages in the saturation buffer and then updates saturation
 * @param chargeMessage whether bps message was charge enable (1) or disable (-1)
*/
static void updateSaturation(int8_t chargeMessage){
    // Replace oldest message with new charge message and update index for oldest message
    chargeMsgBuffer[oldestMsgIdx] = chargeMessage;
    oldestMsgIdx = (oldestMsgIdx + 1) % SAT_BUF_LENGTH;

    // Calculate the new saturation value by assigning weightings from 1 to buffer length
    // in order of oldest to newest
    int newSaturation = 0;
    for (uint8_t i = 0; i < SAT_BUF_LENGTH; i++){
        newSaturation += chargeMsgBuffer[(oldestMsgIdx + i) % SAT_BUF_LENGTH] * (i + 1);
    }
    chargeMsgSaturation = newSaturation;
}

// helper function to call if charging should be disabled
static inline void chargingDisable(void) {
    // mark regen as disabled
    regenEnable = false;

    //kill contactors 
    Contactors_Set(ARRAY_CONTACTOR, OFF, true);
    Contactors_Set(ARRAY_PRECHARGE, OFF, true);
    
    // turn off the array contactor light
    UpdateDisplay_SetArray(false);

}

// helper function to call if charging should be enabled
static inline void chargingEnable(void) {
    OS_ERR err;
    CPU_TS ts;

    // mark regen as enabled
    regenEnable = true;

    // check if we need to run the precharge sequence to turn on the array
    bool shouldRestartArray = false;

    OSMutexPend(&arrayRestartMutex, 0, OS_OPT_PEND_BLOCKING, &ts, &err);
    assertOSError(OS_READ_CAN_LOC,err);

    // if the array is off and we're not already turning it on, start turning it on
    shouldRestartArray = !restartingArray && (Contactors_Get(ARRAY_CONTACTOR)==OFF);

    // wait for precharge for array 
    if(shouldRestartArray){

            restartingArray = true;

            // Wait to make sure precharge is finished and then restart array
            OSTmrStart(&prechargeDlyTimer, &err);
            assertOSError(OS_READ_CAN_LOC, err);
            
        }
    

    OSMutexPost(&arrayRestartMutex, OS_OPT_NONE, &err);
    assertOSError(OS_READ_CAN_LOC,err);
}

/**
 * @brief Callback function for the precharge delay timer. Waits for precharge and then restarts the array.
 * @param p_tmr pointer to the timer that calls this function, passed by timer
 * @param p_arg pointer to the argument passed by timer
 * 
*/
static void arrayRestart(void *p_tmr, void *p_arg){

    if(regenEnable){    // If regen has been disabled during precharge, we don't want to turn on the main contactor immediately after
        Contactors_Set(ARRAY_CONTACTOR, ON, false);
        UpdateDisplay_SetArray(true);
    }
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
    regenEnable = false;
    
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

    //data struct for CAN message
    CANDATA_t dataBuf;

    OSMutexCreate(&arrayRestartMutex, "array restart mutex", &err);
    assertOSError(OS_READ_CAN_LOC,err);


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
    assertOSError(OS_READ_CAN_LOC, err);
    

    while (1)
    {
        
        //Get any message that BPS Sent us
        ErrorStatus status = CANbus_Read(&dataBuf,CAN_BLOCKING,CARCAN);  
        if(status != SUCCESS) {
            continue;
        }

        switch(dataBuf.ID){ //we got a message
            case CHARGE_ENABLE: { 

                // Restart CAN Watchdog timer
                OSTmrStart(&canWatchTimer, &err);
                assertOSError(OS_READ_CAN_LOC, err);


                if(dataBuf.data[0] == 0){ // If the buffer doesn't contain 1 for enable, turn off RegenEnable and turn array off
                    chargingDisable();
                    updateSaturation(-1); // Update saturation and buffer

                } else { //We got a message of enable with a nonzero value
                    updateSaturation(1);

                    // If the charge message saturation is above the threshold, wait for restart precharge sequence then enable charging.
                    // If we are already precharging/array is on, nothing will be done 
                    if (chargeMsgSaturation >= saturationThreshold){
                        chargingEnable();
                    }
                    
                }
                break;
            }
            case SUPPLEMENTAL_VOLTAGE: {
                UpdateDisplay_SetSBPV(*(uint16_t *) &dataBuf.data); // Receive value in mV
                break;
            }
            case STATE_OF_CHARGE:{
                uint8_t SOC = (*(uint32_t*) &dataBuf.data)/(100000);  // Convert to integer percent
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




