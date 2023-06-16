/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "ReadCarCAN.h"
#include "UpdateDisplay.h"
#include "FaultState.h"
#include "Contactors.h"
#include "Minions.h"
#include "os.h"
#include "os_cfg_app.h"

// Saturation threshold is halfway between 0 and max saturation value (half of summation from one to the number of positions)
#define SATURATION_THRESHOLD (((SAT_BUF_LENGTH + 1) * SAT_BUF_LENGTH) / 4) 

// timer delay constants
#define CAN_WATCH_TMR_DLY_MS 500u
#define CAN_WATCH_TMR_DLY_TMR_TS ((CAN_WATCH_TMR_DLY_MS * OS_CFG_TMR_TASK_RATE_HZ) / (1000u)) //1000 for ms -> s conversion
#define PRECHARGE_DLY_TMR_TS (PRECHARGE_ARRAY_DELAY * OS_CFG_TMR_TASK_RATE_HZ)

// CAN watchdog timer variable
static OS_TMR canWatchTimer;

// prechargeDlyTimer
static OS_TMR prechargeDlyTimer;

// NOTE: This should not be written to anywhere other than ReadCarCAN. If the need arises, a mutex to protect it must be added.
// Indicates whether or not regenerative braking / charging is enabled.
static bool chargeEnable = false;

// Saturation buffer variables
static int8_t chargeMsgBuffer[SAT_BUF_LENGTH];
static int chargeMsgSaturation = 0;
static uint8_t oldestMsgIdx = 0;

// SOC and Supp V
static uint8_t SOC = 0;
static uint32_t SBPV = 0;

// Getter function for chargeEnable
bool ChargeEnable_Get(){
    return chargeEnable;
}

/**
 * @brief adds new messages by overwriting old messages in the saturation buffer and then updates saturation
 * @param chargeMessage whether bps message was charge enable (1) or disable (-1)
*/
static void updateSaturation(int8_t chargeMessage){
    if(chargeMessage == 1){
        chargeEnable = true;
    }else{
        chargeEnable = false;
    }

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

// exception struct callback for charging disable, kills contactor and turns off display
static void callback_chargingDisable(void){ 
    // mark regen as disabled and update saturation
    updateSaturation(-1);

    // Kill contactor
    Contactors_Set(ARRAY_CONTACTOR, OFF, true);

    // Turn off the array contactor display light
    UpdateDisplay_SetArray(false);
}

// Nested function because same function needs to be executed
// however the function declarations require different arguments
static void chargingDisable(void *p_tmr, void *p_arg){
    callback_chargingDisable();
}

/**
 * @brief Callback function for the precharge delay timer. Waits for precharge and then restarts the array.
*/
Minion_Error_t Merr;
static void arrayRestart(void *p_tmr, void *p_arg){
    if(chargeEnable){    // If regen has been disabled during precharge, we don't want to turn on the main contactor immediately after
        Contactors_Set(ARRAY_CONTACTOR, (Minion_Read_Pin(IGN_1, &Merr)), false); // Turn on array contactor if the ign switch lets us
        UpdateDisplay_SetArray(true);
    }
};

void Task_ReadCarCAN(void *p_arg){
    OS_ERR err;

    // data struct for CAN message
    CANDATA_t dataBuf;

    // Create the CAN Watchdog (periodic) timer, which disconnects the array and disables regenerative braking
    // if we do not get a CAN message with the ID Charge_Enable within the desired interval.
    OSTmrCreate(
        &canWatchTimer,
        "CAN Watch Timer",
        0,
        CAN_WATCH_TMR_DLY_TMR_TS,
        OS_OPT_TMR_PERIODIC,
        chargingDisable, 
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

    // Start CAN Watchdog timer
    OSTmrStart(&canWatchTimer, &err);
    assertOSError(OS_READ_CAN_LOC, err);

    while(1){

        // Array Contactor is turned off if Ignition 1 is off
        if(Minion_Read_Pin(IGN_1, &Merr) == OFF){
            Contactors_Set(ARRAY_CONTACTOR, OFF, true);
            assertOSError(OS_MINIONS_LOC, err);
        }
        
        // BPS sent a message
        ErrorStatus status = CANbus_Read(&dataBuf, true, CARCAN);
        if(status != SUCCESS){
            continue;
        }

        switch(dataBuf.ID){
            case BPS_TRIP: {
                // BPS has a fault and we need to enter fault state (probably)
                if(dataBuf.data[0] == 1){ // If buffer contains 1 for a BPS trip, we should enter a nonrecoverable fault

                    Display_Evac(SOC, SBPV);    // Display evacuation message

                    // Create an exception and assert the error
                    // kill contactors and enter a nonrecoverable fault
                    exception_t tripBPSError = {.prio=1, .message="BPS has been tripped", .callback=NULL};
                    assertExceptionError(tripBPSError);
                }
            }
            case CHARGE_ENABLE:{

                // Restart CAN Watchdog timer
                OSTmrStart(&canWatchTimer, &err);
                assertOSError(OS_READ_CAN_LOC, err);

                if(dataBuf.data[0] == 0){

                    // Disable regen and update saturation in callback function
                    exception_t disableCharging = {.prio=2, .message="Charging needs to be disabled", .callback=callback_chargingDisable};
                    assertExceptionError(disableCharging);

                } else {

                    // Enable regen and update saturation
                    updateSaturation(1);                               
                    
                    if(Minion_Read_Pin(IGN_1, &Merr) == ON                                          // Ignition is ON
                        && chargeMsgSaturation >= SATURATION_THRESHOLD                              // Saturation Threshold has be met
                        && (Contactors_Get(ARRAY_CONTACTOR)==OFF)                                   // Array Contactor is OFF
                        && OSTmrStateGet(&prechargeDlyTimer, &err) != OS_TMR_STATE_STOPPED){        // Precharge Delay is not runnning and is not executing one-shot 

                            assertOSError(OS_MINIONS_LOC, err);
                            assertOSError(OS_READ_CAN_LOC, err);

                            // Wait to make sure precharge is finished and then restart array
                            OSTmrStart(&prechargeDlyTimer, &err);
                            assertOSError(OS_READ_CAN_LOC, err);
                    }
                    
                }
                break;
            }
            case SUPPLEMENTAL_VOLTAGE: {
                SBPV = (*(uint16_t *) &dataBuf.data);
                UpdateDisplay_SetSBPV(SBPV); // Receive value in mV
                break;
            }
            case STATE_OF_CHARGE:{
                SOC = (*(uint32_t*) &dataBuf.data)/(100000);  // Convert to integer percent
                UpdateDisplay_SetSOC(SOC);
                break;
            }
            default: 
                break;
        }
        
    }
    
}