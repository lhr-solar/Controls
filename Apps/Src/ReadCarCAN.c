/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "ReadCarCAN.h"
#include "UpdateDisplay.h"
#include "Contactors.h"
#include "Minions.h"
#include "os.h"
#include "os_cfg_app.h"

// Array and Motor saturation threshold is halfway between 0 and max saturation value 
// The max saturation value is half of summation from one to the number of positions.
#define ARRAY_SATURATION_THRESHOLD (((SAT_BUF_LENGTH + 1) * SAT_BUF_LENGTH) / 4) 
#define MOTOR_SATURATION_THRESHOLD (((SAT_BUF_LENGTH + 1) * SAT_BUF_LENGTH) / 4) 

// Timer delay constants
#define CAN_WATCH_TMR_DLY_MS 500u
#define CAN_WATCH_TMR_DLY_TMR_TS ((CAN_WATCH_TMR_DLY_MS * OS_CFG_TMR_TASK_RATE_HZ) / (1000u)) //1000 for ms -> s conversion
#define ARRAY_PRECHARGE_BYPASS_DLY_TMR_TS (PRECHARGE_ARRAY_DELAY * OS_CFG_TMR_TASK_RATE_HZ)
#define MOTOR_CONTROLLER_PRECHARGE_BYPASS_DLY_TMR_TS (PRECHARGE_MOTOR_DELAY * OS_CFG_TMR_TASK_RATE_HZ)

// CAN watchdog timer variable
static OS_TMR canWatchTimer;

// Precharge delay timer variable
static OS_TMR arrayBypassPrechargeDlyTimer;

// Precharge delay timer variable
static OS_TMR motorControllerBypassPrechargeDlyTimer;

// NOTE: This should not be written to anywhere other than ReadCarCAN. If the need arises, a mutex to protect it must be added.
// Indicates whether or not regenerative braking / charging is enabled.
static bool chargeEnable = false; // Enable (High message) of BPS high voltage (HV) array contactor

// BPS HV Array saturation buffer variables
static int8_t HVArrayChargeMsgBuffer[SAT_BUF_LENGTH];
static int8_t HVArrayMsgSaturation = 0;
static uint8_t HVArrayOldestMsgIdx = 0;

// BPS HV Plus/Minus (associated with Control's Motor Controller Contactor) saturation buffer variables
static int8_t HVPlusMinusChargeMsgBuffer[SAT_BUF_LENGTH];
static int8_t HVPlusMinusChargeMsgSaturation = 0;
static uint8_t HVPlusMinusMotorOldestMsgIdx = 0;

// Array ignition (IGN_1) and Motor Controller ignition (IGN_2) pin status
static bool arrayIgnitionStatus = false;
static bool motorControllerIgnitionStatus = false;

// Boolean to indicate precharge status for Array Precharge Bypass Contactor (PBC) and Motor Controller PBC
static bool arrayBypassPrechargeComplete = false;
static bool motorControllerBypassPrechargeComplete = false;

// State of Charge (SOC) and supplemental battery pack voltage (SBPV) value intialization
static uint8_t SOC = 0;
static uint32_t SBPV = 0;

// Error assertion function prototype
static void assertReadCarCANError(ReadCarCAN_error_code_t rcc_err);

// Getter function for charge enable, indicating that battery charging is allowed
bool ChargeEnable_Get(void){
    return chargeEnable;
}

// Getter function for HV Array message saturation
int8_t HVArrayMsgSaturation_Get(){ 
	return HVArrayMsgSaturation;
}

// Getter function for HV Plus/Minus (associated with Control's Motor Controller PBC) message saturation
int8_t PlusMinusMsgSaturation_Get(){ 
	return HVPlusMinusChargeMsgSaturation;
}

// Getter function for array ignition status
bool ArrayIgnitionStatus_Get(void){
    return arrayIgnitionStatus;
}

// Getter function for motor controller ignition status
bool MotorControllerIgnition_Get(void){
    return motorControllerIgnitionStatus;
}

// Getter function for State of Charge (SOC)
uint8_t SOC_Get(){ 
	return SOC;
}

// Getter function for supplemental battery pack voltage (SBPV)
uint32_t SBPV_Get(){ 
	return SBPV;
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
 * @brief Callback function for the precharge delay timer. Waits for precharge and then sets arrayBypassPrechargeComplete to true.
 * @param p_tmr pointer to the timer that calls this function, passed by timer
 * @param p_arg pointer to the argument passed by timer
*/
static void setArrayBypassPrechargeComplete(void *p_tmr, void *p_arg){
    arrayBypassPrechargeComplete = true;
};

/**
 * @brief Callback function for the precharge delay timer. Waits for precharge and then sets motorControllerBypassPrechargeComplete to true.
 * @param p_tmr pointer to the timer that calls this function, passed by timer
 * @param p_arg pointer to the argument passed by timer
*/
static void setMotorControllerBypassPrechargeComplete(void *p_tmr, void *p_arg){
    motorControllerBypassPrechargeComplete = true;
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
 * @brief 
 * @param None
*/
static void updateArrayPrechargeBypassContactor(void){
      
    OS_ERR err;
    if((arrayIgnitionStatus == true  || motorControllerIgnitionStatus == true  )                                             // Ignition is ON
        && HVArrayMsgSaturation >= ARRAY_SATURATION_THRESHOLD                         // Saturation Threshold has be met
        && (Contactors_Get(ARRAY_BYPASS_PRECHARGE_CONTACTOR)== OFF)                                          // Array Contactor is OFF
        && (OSTmrStateGet(&arrayBypassPrechargeDlyTimer, &err) != OS_TMR_STATE_RUNNING)){   // and precharge is currenetly not happening  
            // Asserts error for OS timer start above if conditional was met
            assertOSError(OS_READ_CAN_LOC, err);
            // Wait to make sure precharge is finished and then restart array
            OSTmrStart(&arrayBypassPrechargeDlyTimer, &err);
    }
}

static void updateMotorControllerPrechargeBypassContactor(void){
    OS_ERR err;
    if(motorControllerIgnitionStatus == true
        && HVPlusMinusChargeMsgSaturation >= MOTOR_SATURATION_THRESHOLD
        &&(Contactors_Get(MOTOR_CONTROLLER_BYPASS_PRECHARGE_CONTACTOR)== OFF)
        && (OSTmrStateGet(&motorControllerBypassPrechargeDlyTimer, &err) != OS_TMR_STATE_RUNNING)){
            assertOSError(OS_READ_CAN_LOC, err);
            OSTmrStart(&motorControllerBypassPrechargeDlyTimer, &err);
        }
}

/**
 * @brief adds new messages by overwriting old messages in the saturation buffer and then updates saturation
 * @param chargeMessage whether bps message was charge enable (1) or disable (-1)
*/
static void updateHVArraySaturation(int8_t chargeMessage){
    
    // Replace oldest message with new charge message and update index for oldest message
    HVArrayChargeMsgBuffer[HVArrayOldestMsgIdx] = chargeMessage;
    HVArrayOldestMsgIdx = (HVArrayOldestMsgIdx + 1) % SAT_BUF_LENGTH;

    // Calculate the new saturation value by assigning weightings from 1 to buffer length
    // in order of oldest to newest
    int newSaturation = 0;
    for (uint8_t i = 0; i < SAT_BUF_LENGTH; i++){
        newSaturation += HVArrayChargeMsgBuffer[(HVArrayOldestMsgIdx + i) % SAT_BUF_LENGTH] * (i + 1);
    }
    HVArrayMsgSaturation = newSaturation;

    if(chargeMessage == -1){
        chargeEnable = false;
    }else if(HVArrayMsgSaturation >= ARRAY_SATURATION_THRESHOLD){
        chargeEnable = true;
        updateArrayPrechargeBypassContactor();
    }

}

/**
 * @brief adds new messages by overwriting old messages in the saturation buffer and then updates saturation
 * @param chargeMessage whether bps message was charge enable (1) or disable (-1)
*/
static void updateHVPlusMinusSaturation(int8_t chargeMessage){
    
    // Replace oldest message with new charge message and update index for oldest message
    HVPlusMinusChargeMsgBuffer[HVPlusMinusMotorOldestMsgIdx] = chargeMessage;
    HVPlusMinusMotorOldestMsgIdx = (HVPlusMinusMotorOldestMsgIdx + 1) % SAT_BUF_LENGTH;

    // Calculate the new saturation value by assigning weightings from 1 to buffer length
    // in order of oldest to newest
    int newSaturation = 0;
    for (uint8_t i = 0; i < SAT_BUF_LENGTH; i++){
        newSaturation += HVPlusMinusChargeMsgBuffer[(HVPlusMinusMotorOldestMsgIdx + i) % SAT_BUF_LENGTH] * (i + 1);
    }
    HVPlusMinusChargeMsgSaturation = newSaturation;

    if(chargeMessage == 1){
        updateMotorControllerPrechargeBypassContactor();
    }
}

/**
 * @brief Turns array and motor controller precharge bypass contactor ON/OFF based on ignition and precharge status
 * @param None
*/
 void updatePrechargeContactors(void){
    Minion_Error_t Merr;

    arrayIgnitionStatus = (!Minion_Read_Pin(IGN_1, &Merr));
    motorControllerIgnitionStatus = (!Minion_Read_Pin(IGN_2, &Merr));

    if(arrayIgnitionStatus == true && motorControllerIgnitionStatus == false){
        if(arrayBypassPrechargeComplete == true && chargeEnable == true){
            Contactors_Set(ARRAY_BYPASS_PRECHARGE_CONTACTOR, ON, true); // Turn on
            UpdateDisplay_SetArray(true);
            arrayBypassPrechargeComplete = false; 
        }
        Contactors_Set(MOTOR_CONTROLLER_BYPASS_PRECHARGE_CONTACTOR, OFF, true); 
        UpdateDisplay_SetMotor(false);

    }else if(arrayIgnitionStatus == false && motorControllerIgnitionStatus == true){
        if(arrayBypassPrechargeComplete == true && chargeEnable == true){
            Contactors_Set(ARRAY_BYPASS_PRECHARGE_CONTACTOR, ON, true); 
            UpdateDisplay_SetArray(true);
            arrayBypassPrechargeComplete = false; 
        }

        if(motorControllerBypassPrechargeComplete == true && HVPlusMinusChargeMsgSaturation >= MOTOR_SATURATION_THRESHOLD){
            Contactors_Set(MOTOR_CONTROLLER_BYPASS_PRECHARGE_CONTACTOR, ON, true);
            UpdateDisplay_SetMotor(true);
        }
    }else if(arrayIgnitionStatus == false && motorControllerIgnitionStatus == false){
        Contactors_Set(ARRAY_BYPASS_PRECHARGE_CONTACTOR, OFF, true); // Turn off
        UpdateDisplay_SetArray(false);

        Contactors_Set(MOTOR_CONTROLLER_BYPASS_PRECHARGE_CONTACTOR, OFF, true); // Turn off
        UpdateDisplay_SetMotor(false);

        memset(HVArrayChargeMsgBuffer, -1, sizeof(HVArrayChargeMsgBuffer));
        memset(HVPlusMinusChargeMsgBuffer, -1, sizeof(HVPlusMinusChargeMsgBuffer));

        updateHVArraySaturation(-1);
        updateHVPlusMinusSaturation(-1);
    }else{
        assertReadCarCANError(READCARCAN_ERR_MISSED_MSG);
    }
    // Set precharge complete variable to false if precharge happens again
    motorControllerBypassPrechargeComplete = false; 

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
    assertOSError(OS_READ_CAN_LOC, err);

    OSTmrCreate(
        &arrayBypassPrechargeDlyTimer,
        "Array Bypass Precharge Delay Timer",
        0, 
        ARRAY_PRECHARGE_BYPASS_DLY_TMR_TS,
        OS_OPT_TMR_ONE_SHOT,
        setArrayBypassPrechargeComplete,
        NULL,
        &err
    );
    assertOSError(OS_READ_CAN_LOC, err);

    OSTmrCreate(
        &motorControllerBypassPrechargeDlyTimer,
        "Motor Controller Bypass Precharge Delay Timer",
        0,
        MOTOR_CONTROLLER_PRECHARGE_BYPASS_DLY_TMR_TS,
        OS_OPT_TMR_ONE_SHOT,
        setMotorControllerBypassPrechargeComplete,
        NULL,
        &err
    );
    assertOSError(OS_READ_CAN_LOC, err);

    // Start CAN Watchdog timer
    OSTmrStart(&canWatchTimer, &err);
    assertOSError(OS_READ_CAN_LOC, err);

    while(1){
        OSSemPost(&infoMutex,OS_OPT_POST_NONE,&err);
      
        updatePrechargeContactors(); // Sets array and motor controller bypass ignition (IGN_1, IGN_2) contactor ON/OFF

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

            case BPS_CONTACTOR:{
                // Acknowledge CAN Watchdog timer
                OSTmrStart(&canWatchTimer, &err);
                assertOSError(OS_READ_CAN_LOC, err);

                switch (dataBuf.data[0] & 0b11){ // Masking to get last two bits

                    case 0b00:{ // HV+/- and HV Arr did not recieve enable message
                        disableArrayPrechargeBypassContactor();
                        updateHVPlusMinusSaturation(-1);
                        break;
                    }
                    case 0b01:{ // Only HV Arr recieved enable message
                        updateHVArraySaturation(1);
                        updateHVPlusMinusSaturation(-1);
                        break;
                    }      
                    case 0b10:{ // Only HV +/1 recieved enable message
                        disableArrayPrechargeBypassContactor();
                        updateHVPlusMinusSaturation(1);
                        break;
                    }
                    case 0b11: { // Both recieved enable message
                        updateHVArraySaturation(1);
                        updateHVPlusMinusSaturation(1);
                        break;
                    }
                    default: {  
                        // Should not be able to reach here.
                        break;
                    }
                }
                break;
            // End of BPS Contactor Status Updates
            }

            case SUPPLEMENTAL_VOLTAGE: {
                SBPV = (*(uint16_t *) &dataBuf.data);
                UpdateDisplay_SetSBPV(SBPV); // Receive value in mV
                break;
            }
            case STATE_OF_CHARGE:{
                SOC = (*(uint32_t*) &dataBuf.data)/(1000000);  // Convert to integer percent
                UpdateDisplay_SetSOC(SOC);
                break;
            }
        default: {   // Unhandled CAN message IDs
                break; 
            }   
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
    updateHVArraySaturation(-1);

    // Kill contactor using a direct write to avoid blocking calls when the scheduler is locked
    BSP_GPIO_Write_Pin(CONTACTORS_PORT, ARRAY_PRECHARGE_BYPASS_PIN, false);

    // Check that the contactor was successfully turned off
    bool ret = (bool)Contactors_Get(ARRAY_BYPASS_PRECHARGE_CONTACTOR);

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