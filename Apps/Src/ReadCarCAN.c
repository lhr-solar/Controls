/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "ReadCarCAN.h"
#include "Display.h"
#include "Contactors.h"
#include "Minions.h"
#include "CAN_Queue.h"
#include "FaultState.h"
#include "os_cfg_app.h"


//MMM CAN watchdog timer variables
static OS_TMR canWatchTimer; // Is it ok to declare this here?
static const int CAN_WATCH_TMR_DLY_MS = 500;
static const int CAN_WATCH_TMR_DLY_TMR_TS = (CAN_WATCH_TMR_DLY_MS / DEF_TIME_NBR_mS_PER_SEC) * OS_CFG_TMR_TASK_RATE_HZ;

//MMM prechargeDlyTimer
static OS_TMR prechargeDlyTimer;
static const int PRECHARGE_DLY_TMR_TS = PRECHARGE_ARRAY_DELAY * OS_CFG_TMR_TASK_RATE_HZ;

//MMM restart array and disable charging semaphonres
static OS_SEM restartArraySem;
static OS_SEM canWatchDisableChargingSem;


// Array restart thread lock
static OS_MUTEX arrayRestartMutex;
static bool restartingArray = false;

/*
static void CANWatchdog_Handler(); //Handler if we stop getting messages */
static void arrayRestart(void *p_tmr, void *p_arg); //handler to turn array back on


// helper function to call if charging should be disabled
static inline void chargingDisable(void) {
    // mark regen as disabled
    RegenEnable = OFF;

    //kill contactors 
    Contactors_Set(ARRAY_CONTACTOR, OFF, SET_CONTACTORS_BLOCKING);
    Contactors_Set(ARRAY_PRECHARGE, OFF, SET_CONTACTORS_BLOCKING);
    
    // turn off the array contactor light
    //Display_SetLight(A_CNCTR, OFF);
}

// helper function to call if charging should be enabled
static inline void chargingEnable(void) {
    OS_ERR err;
    CPU_TS ts;

    // mark regen as enabled
    RegenEnable = ON;

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

    // start precharge for array 
    if(shouldRestartArray){

         if (Contactors_Set(ARRAY_PRECHARGE, ON, SET_CONTACTORS_BLOCKING) == SUCCESS) {    // only run through precharge sequence if we successfully start precharge
            restartingArray = true;

            // Wait to make sure precharge is finished and then restart array
            OSTmrStart(&prechargeDlyTimer, &err);
            
        }
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

    if(!RegenEnable){    // If regen enable has been disabled during precharge, we don't want to turn on the main contactor immediately after
        restartingArray = false;
    }
    else {
        // Try to turn on array, but only if we can successfully turn off precharge
        if(Contactors_Set(ARRAY_PRECHARGE, OFF, SET_CONTACTORS_NON_BLOCKING) == SUCCESS){

            Contactors_Set(ARRAY_CONTACTOR, ON, SET_CONTACTORS_NON_BLOCKING);

        }
    }

        Lights_Set(A_CNCTR, ON);

        // done restarting the array
        restartingArray = false;

};

/**
 * @brief Disables charging and increments the trip counter
 * when canWatchTimer hits 0 (when charge enable messages are missed)
 * @param p_tmr pointer to the timer that calls this function, passed by timer
 * @param p_arg pointer to the argument passed by timer
*/
void canWatchTimerCallback (void *p_tmr, void *p_arg){  //Probably needs its timer arguments
    OS_ERR err;

    // mark regen as disabled
    RegenEnable = OFF;
    
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


    //Create the CAN Watchdog (periodic) timer
    /**
     * @brief This function is the handler thread for the CANWatchdog timer. It disconnects the array and disables regenerative braking if we do not get
     * a CAN message with the ID Charge_Enable within the desired interval.
    */
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
            case CHARGE_ENABLE: { //M Don't think we need msg_received

                //Start CAN Watchdog timer
                OSTmrStart(&canWatchTimer, &err);

                if(buffer[0] == 0){ // If the buffer doesn't contain 1 for enable, turn off RegenEnable and turn array off
                    chargingDisable();
                } else {
                    //We got a message of enable with a nonzero value, turn on Regen, If we are already in precharge / array is on, do nothing. 
                    //If not, initiate precharge and restart sequence. 
                    chargingEnable();
                }
                break;
            }
            case SUPPLEMENTAL_VOLTAGE: { //TODO: do some telemetry stuff with other messages 
                SupplementalVoltage = *(uint16_t *) &buffer;
                break;
            }
            case STATE_OF_CHARGE:{
                StateOfCharge = *(uint32_t*) &buffer; //get the 32 bit message and store it
                break;
            }
            default: 
                break;
        }
    }
}







