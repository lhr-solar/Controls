/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "ReadCarCAN.h"
#include "UpdateDisplay.h"
#include "Contactors.h"
#include "Minions.h"
#include "os.h"
#include "os_cfg_app.h"

#ifndef __TEST_READCARCAN
#define RCC_SCOPE
#else 
#define RCC_SCOPE static
#endif

// Saturation threshold is halfway between 0 and max saturation value (half of summation from one to the number of positions)
#define SATURATION_THRESHOLD (((SAT_BUF_LENGTH + 1) * SAT_BUF_LENGTH) / 4) 

// timer delay constants
#define CAN_WATCH_TMR_DLY_MS 500u
#define CAN_WATCH_TMR_DLY_TMR_TS ((CAN_WATCH_TMR_DLY_MS * OS_CFG_TMR_TASK_RATE_HZ) / (1000u)) //1000 for ms -> s conversion
#define PRECHARGE_DLY_TMR_TS (PRECHARGE_ARRAY_DELAY * OS_CFG_TMR_TASK_RATE_HZ)

// CAN watchdog timer variable
static OS_TMR canWatchTimer;

// Precharge delay timer variable
static OS_TMR prechargeDlyTimer;

// NOTE: This should not be written to anywhere other than ReadCarCAN. If the need arises, a mutex to protect it must be added.
// Indicates whether or not regenerative braking / charging is enabled.
RCC_SCOPE bool chargeEnable = false;

// Saturation buffer variables
static int8_t chargeMsgBuffer[SAT_BUF_LENGTH];
int chargeMsgSaturation = 0;
static uint8_t oldestMsgIdx = 0;

// Array ignition (IGN_1) pin status
#ifdef __TEST_READCARCAN
static bool arrayIgnitionStatus = false; // Can't test logic if ignition is always off
#else
static bool arrayIgnitionStatus = false;
#endif

// Boolean to indicate precharge status
bool prechargeComplete = false;

// SOC and Supp V
uint8_t SOC = 0;
uint32_t SBPV = 0;

// Error assertion function prototype
static void assertReadCarCANError(ReadCarCAN_error_code_t rcc_err);

// Getter function for chargeEnable
bool ChargeEnable_Get(void){
    return chargeEnable;
}

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

    if(chargeMessage == -1){
        chargeEnable = false;
    }else if(chargeMsgSaturation >= SATURATION_THRESHOLD){
        chargeEnable = true;
    }
}

/**
 * @brief Nested function as the same function needs to be executed however the timer requires different parameters
 * @param p_tmr pointer to the timer that calls this function, passed by timer
 * @param p_arg pointer to the argument passed by timer
*/
static void callbackCANWatchdog(void *p_tmr, void *p_arg){
    assertReadCarCANError(READCARCAN_ERR_MISSED_MSG);
}

/**
 * @brief Callback function for the precharge delay timer. Waits for precharge and then restarts the array.
 * @param p_tmr pointer to the timer that calls this function, passed by timer
 * @param p_arg pointer to the argument passed by timer
*/
static void arrayRestart(void *p_tmr, void *p_arg){
    prechargeComplete = true;
};

/**
 * @brief Callback function for the precharge delay timer. Waits for precharge and then restarts the array.
 * @param p_tmr pointer to the timer that calls this function, passed by timer
 * @param p_arg pointer to the argument passed by timer
*/
static void ignitionStatusLogic(){
   // Array Contactor is turned off if Ignition 1 is off
        if(arrayIgnitionStatus == false){
            Contactors_Set(ARRAY_CONTACTOR, OFF, true);

            // Update Display
            UpdateDisplay_SetArray(false);
        }
        if(prechargeComplete == true){    // If regen has been disabled during precharge, we don't want to turn on the main contactor immediately after
        Contactors_Set(ARRAY_CONTACTOR, ON, false); // Turn on array contactor 
    }
    prechargeComplete = false;
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
        CAN_WATCH_TMR_DLY_TMR_TS, // Initial delay equal to the period since 0 doesn't seem to work
        CAN_WATCH_TMR_DLY_TMR_TS, 
        OS_OPT_TMR_PERIODIC,
        callbackCANWatchdog, 
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

    #ifndef __TEST_READCARCAN
  //  Minion_Error_t Merr; // Not used when testing since ignition won't be changed
    #endif

  Minion_Error_t Merr;
    while(1){
      
        arrayIgnitionStatus = Minion_Read_Pin(IGN_1, &Merr); // Don't check ignition if just testing

        
        

ignitionStatusLogic();

        // BPS sent a message
        ErrorStatus status = CANbus_Read(&dataBuf, true, CARCAN);
        if(status != SUCCESS){
            continue;
        }




        switch(dataBuf.ID){
            case BPS_TRIP: { // BPS has a fault and we need to enter fault state
                
                // Assert the error to kill contactors, display evacuation message, and enter a nonrecoverable fault
                assertReadCarCANError(READCARCAN_ERR_BPS_TRIP);
                
            }
            case CHARGE_ENABLE:{

                // Acknowledge CAN Watchdog timer
                OSTmrStart(&canWatchTimer, &err);
                assertOSError(OS_READ_CAN_LOC, err);

                if(dataBuf.data[0] == 0){

                    // Assert error to disable regen and update saturation in callback function
                    assertReadCarCANError(READCARCAN_ERR_CHARGE_DISABLE);
                    
                    // Turn off the array contactor display light
                    UpdateDisplay_SetArray(false); // Can assume contactor turned off or else this won't be reached

                } else {

                    // Enable regen and update saturation
                    updateSaturation(1);                               
                    
                    if(arrayIgnitionStatus == true                                                  // Ignition is ON
                        && chargeMsgSaturation >= SATURATION_THRESHOLD                              // Saturation Threshold has be met
                        && (Contactors_Get(ARRAY_CONTACTOR)==OFF)                                   // Array Contactor is OFF
                        && ((OSTmrStateGet(&prechargeDlyTimer, &err) == OS_TMR_STATE_STOPPED)       // Precharge Delay is not running
                        || (OSTmrStateGet(&prechargeDlyTimer, &err) == OS_TMR_STATE_COMPLETED))){   // or has already completed
                            
                            Contactors_Set(ARRAY_CONTACTOR, ON, true);
                            // Wait to make sure precharge is finished and then restart array
                            assertOSError(OS_READ_CAN_LOC, err);
                            OSTmrStart(&prechargeDlyTimer, &err);
                    }
                    
                    // Asserts error for OS timer start above if conditional was met
                    // Asserts error for OS timer state get if conditional wasn't met 
                    assertOSError(OS_READ_CAN_LOC, err);

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

/**
 * Error handler functions
 * Passed as callback functions to the main assertTaskError function by assertReadCarCANError
*/

/**
 * @brief error handler callback for disabling charging, 
 * kills contactor and turns off display
 */ 
static void handler_ReadCarCAN_chargeDisable(void) {

    // mark regen as disabled and update saturation
    updateSaturation(-1);

    // Kill contactor using a direct write to avoid blocking calls when the scheduler is locked
    BSP_GPIO_Write_Pin(CONTACTORS_PORT, ARRAY_CONTACTOR_PIN, false);

    // Check that the contactor was successfully turned off
    bool ret = (bool)Contactors_Get(ARRAY_CONTACTOR);

    if(ret != false) { // Contactor failed to turn off; display the evac screen and infinite loop
         Display_Evac(SOC, SBPV);
         while(1){;}
    }
  
}

/**
 * @brief error handler function to display the evac screen if we get a BPS trip message.
 * Callbacks happen after displaying the fault, so this screen won't get overwritten
 */ 
static void handler_ReadCarCAN_BPSTrip(void) {
    chargeEnable = false; // Not really necesssary but makes inspection less confusing
	Display_Evac(SOC, SBPV);   // Display evacuation screen
}


/**
 * @brief error assertion function for ReadCarCAN, used to disable charging and handle BPS trip messages
 * Stores the error code and calls assertTaskError with the appropriate parameters and callback handler
 * @param  rcc_err error code to specify the issue encountered
 */
static void assertReadCarCANError(ReadCarCAN_error_code_t rcc_err){   
	Error_ReadCarCAN = (error_code_t) rcc_err; // Store error code for inspection

    switch (rcc_err) {
            case READCARCAN_ERR_NONE: 
                break;

            case READCARCAN_ERR_CHARGE_DISABLE: // Received a charge disable msg and need to turn off array contactor
                assertTaskError(OS_READ_CAN_LOC, READCARCAN_ERR_CHARGE_DISABLE, handler_ReadCarCAN_chargeDisable, OPT_LOCK_SCHED, OPT_RECOV);
                break;

            case READCARCAN_ERR_MISSED_MSG: // Missed message- treat as charging disable message
                assertTaskError(OS_READ_CAN_LOC, READCARCAN_ERR_MISSED_MSG, handler_ReadCarCAN_chargeDisable, OPT_LOCK_SCHED, OPT_RECOV);
                break;

            case READCARCAN_ERR_BPS_TRIP: // Received a BPS trip msg (0 or 1), need to shut down car and infinite loop
                assertTaskError(OS_READ_CAN_LOC, READCARCAN_ERR_BPS_TRIP, handler_ReadCarCAN_BPSTrip, OPT_LOCK_SCHED, OPT_NONRECOV);
                break;
            
            default:
                break;
	}

	Error_ReadCarCAN = READCARCAN_ERR_NONE; // Clear the error after handling it
}