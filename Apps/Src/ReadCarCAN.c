/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file ReadCarCAN.c
 * @brief 
 * 
 */

#include "ReadCarCAN.h"
#include "UpdateDisplay.h"
#include "Contactors.h"
#include "Minions.h"
#include "os.h"
#include "os_cfg_app.h"
#include "Display.h"

// Length of the array and motor PBC saturation buffers
#define SAT_BUF_LENGTH 5 

// The Array/Motor Controller Saturation Threshold is used to determine if Controls has 
//      received a sufficient number of BPS's HV Array/Plus-Minus Enable Messages.
//      BPS Array and Plus/Minus saturation threshold is halfway between 0 and max saturation value.
#define ARRAY_SATURATION_THRESHOLD (((SAT_BUF_LENGTH + 1) * SAT_BUF_LENGTH) / 4) 
#define PLUS_MINUS_SATURATION_THRESHOLD (((SAT_BUF_LENGTH + 1) * SAT_BUF_LENGTH) / 4) 

// Timer delay constants
#define CAN_WATCH_TMR_DLY_MS 500u // 500 ms
#define CAN_WATCH_TMR_DLY_TMR_TS ((CAN_WATCH_TMR_DLY_MS * OS_CFG_TMR_TASK_RATE_HZ) / (1000u)) // 1000 for ms -> s conversion

// Precharge Delay times in milliseconds
#define PRECHARGE_PLUS_MINUS_DELAY 100u	// 100 ms, as this the smallest time delay that the RTOS can work with
#define PRECHARGE_ARRAY_DELAY 100u     	// 100 ms
#define ARRAY_PRECHARGE_BYPASS_DLY_TMR_TS ((PRECHARGE_ARRAY_DELAY * OS_CFG_TMR_TASK_RATE_HZ) / (1000u)) 
#define MOTOR_CONTROLLER_PRECHARGE_BYPASS_DLY_TMR_TS ((PRECHARGE_PLUS_MINUS_DELAY * OS_CFG_TMR_TASK_RATE_HZ) / (1000u))

// High Voltage BPS Contactor bit mapping
#define HV_ARRAY_CONTACTOR_BIT 0b001
#define HV_MINUS_CONTACTOR_BIT 0b010
#define HV_PLUS_CONTACTOR_BIT 0b100

// Saturation messages
#define DISABLE_SATURATION_MSG -1
#define ENABLE_SATURATION_MSG 1

// State of Charge scalar to scale it to correct fix point
#define SOC_SCALER 1000000

// CAN watchdog timer variable
static OS_TMR canWatchTimer;

// Array precharge bypass contactor delay timer variable
static OS_TMR arrayPBCDlyTimer;

// Motor controller precharge bypass contactor delay timer variable
static OS_TMR motorControllerPBCDlyTimer;

// NOTE: This should not be written to anywhere other than ReadCarCAN. If the need arises, a mutex to protect it must be added.
// Indicates whether or not regenerative braking / charging is enabled.
static bool chargeEnable = false; // Enable (High message) of BPS high voltage (HV) array contactor

// BPS HV Array saturation buffer variables
static int8_t HVArrayChargeMsgBuffer[SAT_BUF_LENGTH];
static int8_t HVArrayMsgSaturation = 0;
static uint8_t HVArrayOldestMsgIdx = 0;

// BPS HV Motor Controller saturation buffer variables
static int8_t HVPlusMinusChargeMsgBuffer[SAT_BUF_LENGTH];
static int8_t HVPlusMinusChargeMsgSaturation = 0;
static uint8_t HVPlusMinusMotorOldestMsgIdx = 0;

// Array ignition (IGN_1) and Motor Controller ignition (IGN_2) pin status
static bool arrIgnStatus = false;
static bool mcIgnStatus = false;

// Boolean to indicate precharge status for Array Precharge Bypass Contactor (PBC) and Motor Controller PBC
static bool arrPBCComplete = false;
static bool mcPBCComplete = false;

// State of Charge (SOC) and supplemental battery pack voltage (SBPV) value intialization
static uint8_t SOC = 0;
static uint32_t SBPV = 0;

// Error assertion function prototype
static void assertReadCarCANError(ReadCarCAN_error_code_t rcc_err);

// Getter function for charge enable, indicating that battery charging is allowed
bool ChargeEnable_Get(void){
    return chargeEnable;
}

/**
 * @brief Nested function as the same function needs to be cexecuted however the timer requires different parameters
 * @param p_tmr pointer to the timer that calls this function, passed by timer
 * @param p_arg pointer to the argument passed by timer
*/
static void callbackCANWatchdog(void *p_tmr, void *p_arg){
   assertReadCarCANError(READCARCAN_ERR_MISSED_MSG);
}

/**
 * @brief Callback function for the precharge delay timer. Waits for precharge and then sets arrPBCComplete to true.
 * @param p_tmr pointer to the timer that calls this function, passed by timer
 * @param p_arg pointer to the argument passed by timer
*/
static void setArrayBypassPrechargeComplete(void *p_tmr, void *p_arg){
    arrPBCComplete = true;
};

/**
 * @brief Callback function for the precharge delay timer. Waits for precharge and then sets mcPBCComplete to true.
 * @param p_tmr pointer to the timer that calls this function, passed by timer
 * @param p_arg pointer to the argument passed by timer
*/
static void setMotorControllerBypassPrechargeComplete(void *p_tmr, void *p_arg){
    mcPBCComplete = true;
};

/**
 * @brief Disables Array Precharge Bypass Contactor (PBC) by asserting an error. Also updates display for Array PBC to be open.
 * @param None
*/
static void disableArrayPrechargeBypassContactor(void){
    // Assert error to disable regen and update saturation in callback function
    assertReadCarCANError(READCARCAN_ERR_CHARGE_DISABLE);
    // Turn off the array contactor display light
    UpdateDisplay_SetArray(false); // Can assume contactor turned off or else this won't be reached
}

/**
 * @brief Turns array PBC on if conditional meets ignition status, saturation threshold, array PCB to be off, 
 *          and a non-running precharge delay timer.
 * @param None
*/
static void updateArrayPrechargeBypassContactor(void){ 
    OS_ERR err = OS_ERR_NONE;
    if((arrIgnStatus || mcIgnStatus)                    // Ignition is ON
        && HVArrayMsgSaturation >= ARRAY_SATURATION_THRESHOLD                                       // Saturation Threshold has be met
        && (Contactors_Get(ARRAY_PRECHARGE_BYPASS_CONTACTOR) == OFF)                                // Array PBC is OFF
        && (OSTmrStateGet(&arrayPBCDlyTimer, &err) != OS_TMR_STATE_RUNNING)){           // and precharge is currently not happening  
            // Asserts error for OS timer state above if conditional was met
            assertOSError(err);
            // Wait to make sure precharge is finished and then restart array
            OSTmrStart(&arrayPBCDlyTimer, &err);
            // Asserts error for OS timer start above if conditional was met
            assertOSError(err);
    }
    assertOSError(err);
}


/**
 * @brief Turns array PBC on if conditional meets ignition status, saturation threshold, motor PBC to be off, 
 *          and a non-running precharge delay timer.
 * @param None
*/
static void updateMCPBC(void){
    OS_ERR err = OS_ERR_NONE;
    if(mcIgnStatus == true                                                                // Ignition is ON
        && HVPlusMinusChargeMsgSaturation >= PLUS_MINUS_SATURATION_THRESHOLD                                // Saturation Threshold has be met
        &&(Contactors_Get(MOTOR_CONTROLLER_PRECHARGE_BYPASS_CONTACTOR) == OFF)                              // Motor Controller PBC is OFF
        && (OSTmrStateGet(&motorControllerPBCDlyTimer, &err) != OS_TMR_STATE_RUNNING)){         // and precharge is currently not happening  
            // Asserts error for OS timer state above if conditional was met
            assertOSError(err);
            // Wait to make sure precharge is finished and then restart array
            OSTmrStart(&motorControllerPBCDlyTimer, &err);
            // Asserts error for OS timer start above if conditional was met
            assertOSError(err);
        }
    assertOSError(err);
}

/**
 * @brief adds new messages by overwriting old messages in the saturation buffer and then updates saturation
 * @param messageState whether bps message was enable (1) or disable (-1)
*/
static void updateHVArraySaturation(int8_t messageState){
    
    // Replace oldest message with new charge message and update index for oldest message
    HVArrayChargeMsgBuffer[HVArrayOldestMsgIdx] = messageState;
    HVArrayOldestMsgIdx = (HVArrayOldestMsgIdx + 1) % SAT_BUF_LENGTH;

    // Calculate the new saturation value by assigning weightings from 1 to buffer length
    // in order of oldest to newest
    int newSaturation = 0;
    for (uint8_t i = 0; i < SAT_BUF_LENGTH; i++){
        newSaturation += HVArrayChargeMsgBuffer[(HVArrayOldestMsgIdx + i) % SAT_BUF_LENGTH] * (i + 1);
    }
    HVArrayMsgSaturation = newSaturation;

    if(messageState == -1){
        chargeEnable = false;
    }else if(HVArrayMsgSaturation >= ARRAY_SATURATION_THRESHOLD){
        chargeEnable = true;
        updateArrayPrechargeBypassContactor();
    }

}

/**
 * @brief adds new messages by overwriting old messages in the saturation buffer and then updates saturation
 * @param messageState whether bps message was  enable (1) or disable (-1)
*/
static void updateHVPlusMinusSaturation(int8_t messageState){
    
    // Replace oldest message with new charge message and update index for oldest message
    HVPlusMinusChargeMsgBuffer[HVPlusMinusMotorOldestMsgIdx] = messageState;
    HVPlusMinusMotorOldestMsgIdx = (HVPlusMinusMotorOldestMsgIdx + 1) % SAT_BUF_LENGTH;

    // Calculate the new saturation value by assigning weightings from 1 to buffer length
    // in order of oldest to newest
    int newSaturation = 0;
    for (uint8_t i = 0; i < SAT_BUF_LENGTH; i++){
        newSaturation += HVPlusMinusChargeMsgBuffer[(HVPlusMinusMotorOldestMsgIdx + i) % SAT_BUF_LENGTH] * (i + 1);
    }
    HVPlusMinusChargeMsgSaturation = newSaturation;

    if(messageState == 1){
        updateMCPBC();
    }
}

/**
 * @brief Helper to turn arrayPBCOn if arrayBypassPrecharge is completed and charging is enabled
 * @param None
*/
 void turnArrayPBCOn(void){
    if(arrPBCComplete && chargeEnable){
            Contactors_Set(ARRAY_PRECHARGE_BYPASS_CONTACTOR, ON, true); // Turn on
            UpdateDisplay_SetArray(true);
            arrPBCComplete = false; 
        }
 }

/**
 * @brief Helper to turn motorControllerPBCOn if motorControllerBypassPrecharge is completed and threshold is reached
 * @param None
*/
 void turnMotorControllerPBCOn(void){
    if(mcPBCComplete){
            Contactors_Set(MOTOR_CONTROLLER_PRECHARGE_BYPASS_CONTACTOR, ON, true);
            UpdateDisplay_SetMotor(true);  
        }
 }

/**
 * @brief Helper to turn motorControllerPBCOff if ignition is not turned to motor or motor controller threshold isn't reached. 
 * @param None
*/
 void turnMotorControllerPBCOff(void){
    Contactors_Set(MOTOR_CONTROLLER_PRECHARGE_BYPASS_CONTACTOR, OFF, true); 
    UpdateDisplay_SetMotor(false);
 }


/**
 * @brief Turns array and motor controller PBC ON/OFF based on ignition and precharge status
 * @param None
*/
 void updatePrechargeContactors(void){

    arrIgnStatus = (!Minions_Read(IGN_1));
    mcIgnStatus = (!Minions_Read(IGN_2));

    // Logic helper in cases both are off or (impossible) on 
    bool bothStatusOff = !mcIgnStatus && !arrIgnStatus;
    bool bothStatusOn = mcIgnStatus && arrIgnStatus;

    if(bothStatusOff || bothStatusOn){
        assertReadCarCANError(READCARCAN_ERR_DISABLE_CONTACTORS_MSG); // Turn Array and Motor Controller PBC off using error assert

    }else if(!mcIgnStatus && arrIgnStatus){
        turnArrayPBCOn();               // Turn Array PBC On, if permitted
        turnMotorControllerPBCOff();    // Turn Motor Controller PBC Off

    }else if(mcIgnStatus && !arrIgnStatus){
        turnArrayPBCOn();                                                           // Turn Array PBC On, if permitted
        if(HVPlusMinusChargeMsgSaturation >= PLUS_MINUS_SATURATION_THRESHOLD){      // Turn Motor Controller PBC On, if threshold is reached
            turnMotorControllerPBCOn();
        }else{
            turnMotorControllerPBCOff();
        }
    }

    // Set precharge complete variable to false if precharge happens again
    arrPBCComplete = false;
    mcPBCComplete = false; 
}

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
    assertOSError(err);

    OSTmrCreate(
        &arrayPBCDlyTimer,
        "Array Bypass Precharge Delay Timer",
        0,
        ARRAY_PRECHARGE_BYPASS_DLY_TMR_TS,
        OS_OPT_TMR_ONE_SHOT,
        setArrayBypassPrechargeComplete,
        NULL,
        &err
    );
    assertOSError(err);

    OSTmrCreate(
        &motorControllerPBCDlyTimer,
        "Motor Controller Bypass Precharge Delay Timer",
        0,
        MOTOR_CONTROLLER_PRECHARGE_BYPASS_DLY_TMR_TS,
        OS_OPT_TMR_ONE_SHOT,
        setMotorControllerBypassPrechargeComplete,
        NULL,
        &err
    );
    assertOSError(err);

    // Start CAN Watchdog timer
    OSTmrStart(&canWatchTimer, &err);
    assertOSError(err);

    // Fills buffers with disable messages
    memset(HVArrayChargeMsgBuffer, DISABLE_SATURATION_MSG, sizeof(HVArrayChargeMsgBuffer));
    memset(HVPlusMinusChargeMsgBuffer, DISABLE_SATURATION_MSG, sizeof(HVPlusMinusChargeMsgBuffer));

    while(1){
              
        updatePrechargeContactors(); // Sets array and motor controller PBC if all conditions (PBC Status, Threshold, Precharge Complete) permit

        // BPS sent a message
        ErrorStatus status = CANbus_Read(&dataBuf, true, CARCAN);
        if(status != SUCCESS){
            continue;
        }

        switch(dataBuf.ID){  // Switch case based on BPS msg received
            case BPS_TRIP: { // BPS has a fault and we need to enter fault state 

                // kill contactors and enter a nonrecoverable fault
                assertReadCarCANError(READCARCAN_ERR_BPS_TRIP);
            }
            case BPS_CONTACTOR: { 

                OSTmrStart(&canWatchTimer, &err); // Restart CAN Watchdog timer for BPS Contactor msg
                assertOSError(err); 
                
                // Retrieving HV contactor statuses using bit mapping
                // Bitwise to get HV Plus and Minus, and then &&ing to ensure both are on
                bool HVPlusMinusStatus = (bool)((dataBuf.data[0] & HV_PLUS_CONTACTOR_BIT) && (dataBuf.data[0] & HV_MINUS_CONTACTOR_BIT));
                // Bitwise to get HV Array
                bool HVArrayStatus = (bool)(dataBuf.data[0] & HV_ARRAY_CONTACTOR_BIT);

                // Update HV Array and HV Plus/Minus saturations based on the respective statuses
                HVArrayStatus ? updateHVArraySaturation(ENABLE_SATURATION_MSG) : disableArrayPrechargeBypassContactor();
                HVPlusMinusStatus ? updateHVPlusMinusSaturation(ENABLE_SATURATION_MSG) : updateHVPlusMinusSaturation(DISABLE_SATURATION_MSG);

                break; // End of BPS Contactor Status Updates
            }

            case SUPPLEMENTAL_VOLTAGE: {
                SBPV = (*(uint16_t *) &dataBuf.data);
                UpdateDisplay_SetSBPV(SBPV); // Receive value in mV
                break;
            }
            case STATE_OF_CHARGE:{
                SOC = (*(uint32_t*) &dataBuf.data)/(SOC_SCALER);  // Convert to integer percent
                UpdateDisplay_SetSOC(SOC);
                break;
            }
        default: {  
            break; // Unhandled CAN message IDs, do nothing
            }   
        }
    }
}

/**
 * @brief error handler callback for disabling charging, 
 * kills contactor and turns off display
 */ 
static void handler_ReadCarCAN_chargeDisable(void) {
    // Fills buffers with disable messages
    memset(HVArrayChargeMsgBuffer, DISABLE_SATURATION_MSG, sizeof(HVArrayChargeMsgBuffer));

    // mark regen as disabled and update saturation
    updateHVArraySaturation(DISABLE_SATURATION_MSG);

    // Kill contactor using a direct write to avoid blocking calls when the scheduler is locked
    BSP_GPIO_Write_Pin(CONTACTORS_PORT, ARRAY_PRECHARGE_BYPASS_PIN, false);

    // Check that the contactor was successfully turned off
    bool ret = (bool)Contactors_Get(ARRAY_PRECHARGE_BYPASS_CONTACTOR);

    if(ret != false) { // Contactor failed to turn off; display the evac screen and infinite loop
        Display_Evac(SOC, SBPV);
        while(1){;}
    }
}

/**
 * @brief error handler callback for disabling charging, 
 * kills contactor and turns off display
 */ 
static void handler_ReadCarCAN_contactorsDisable(void) {

    // Mark regen as disabled and update saturation
    updateHVArraySaturation(DISABLE_SATURATION_MSG);
    updateHVPlusMinusSaturation(DISABLE_SATURATION_MSG);

    // Kill contactor using a direct write to avoid blocking calls when the scheduler is locked
    BSP_GPIO_Write_Pin(CONTACTORS_PORT, ARRAY_PRECHARGE_BYPASS_PIN, false);
    BSP_GPIO_Write_Pin(CONTACTORS_PORT, MOTOR_CONTROLLER_PRECHARGE_BYPASS_PIN, false);

    // Fills buffers with disable messages
    memset(HVArrayChargeMsgBuffer, DISABLE_SATURATION_MSG, sizeof(HVArrayChargeMsgBuffer));
    memset(HVPlusMinusChargeMsgBuffer, DISABLE_SATURATION_MSG, sizeof(HVPlusMinusChargeMsgBuffer));

    // Updates the saturation with disable
    updateHVArraySaturation(DISABLE_SATURATION_MSG);
    updateHVPlusMinusSaturation(DISABLE_SATURATION_MSG);

    // Check that the contactor was successfully turned off
    bool ret = (bool)Contactors_Get(ARRAY_PRECHARGE_BYPASS_CONTACTOR) || (bool)Contactors_Get(MOTOR_CONTROLLER_PRECHARGE_BYPASS_CONTACTOR);

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
    chargeEnable = false;       // Not really necessary but makes inspection less confusing
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
                throwTaskError(Error_ReadCarCAN, handler_ReadCarCAN_chargeDisable, OPT_LOCK_SCHED, OPT_RECOV);
                break;

            case READCARCAN_ERR_MISSED_MSG: // Missed message- turn off array and motor controller PBC
                throwTaskError(Error_ReadCarCAN, handler_ReadCarCAN_contactorsDisable, OPT_LOCK_SCHED, OPT_RECOV);
                break;

            case READCARCAN_ERR_DISABLE_CONTACTORS_MSG: // Ignition turned to off or turned to last two simultaneously 
                throwTaskError(Error_ReadCarCAN, handler_ReadCarCAN_contactorsDisable, OPT_LOCK_SCHED, OPT_RECOV);
                break;

            case READCARCAN_ERR_BPS_TRIP: // Received a BPS trip msg (0 or 1), need to shut down car and infinite loop
                throwTaskError(Error_ReadCarCAN, handler_ReadCarCAN_BPSTrip, OPT_LOCK_SCHED, OPT_NONRECOV);
                break;
            
            default:
                break;
	}

	Error_ReadCarCAN = READCARCAN_ERR_NONE; // Clear the error after handling it
}