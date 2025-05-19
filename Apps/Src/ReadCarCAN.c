/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file ReadCarCAN.c
 * @brief
 *
 */

#include "ReadCarCAN.h"
#include "UpdateDisplay.h"
#include "Contactors.h"
#include "Ignition.h"
#include "os.h"
#include "StatusLeds.h"
#include "os_cfg_app.h"
#include "Display.h"
#include "DebugIO.h"
#include "daybreak_pins.h"

// Uncomment this to remove CAN watchdog timers for BPS_CONTACTOR and CONTACTOR_SENSE messages
#define NODOGS

// Timer delay constants
#define CAN_WATCH_TMR_DLY_MS 1000u                                                             // 500 ms
#define CAN_WATCH_TMR_DLY_TMR_TS ((CAN_WATCH_TMR_DLY_MS * OS_CFG_TMR_TASK_RATE_HZ) / (1000u)) // 1000 for ms -> s conversion

// High Voltage BPS Contactor bit mapping
#define HV_ARRAY_CONTACTOR_BIT 1 // 0b001
#define HV_MINUS_CONTACTOR_BIT 2 // 0b010
#define HV_PLUS_CONTACTOR_BIT 4  // 0b100

// State of Charge scalar to scale it to correct fixed point
#define SOC_SCALER 1000000

#ifndef NODOGS
// BPS CAN watchdog timer variable
static OS_TMR canWatchTimer;

// Active Precharge CAN watchdog timer variable
static OS_TMR prechargeCanWatchTimer;
#endif

// State of Charge (SOC) and supplemental battery pack voltage (SBPV) value intialization
static uint32_t SOC = 0;
static uint32_t SBPV = 0;

// Error assertion function prototype
// static void assertReadCarCANError(ReadCarCAN_error_code_t rcc_err);


void display_err_failed_recovery(void) {
    UpdateDisplay_SetSBPV(SBPV);
    UpdateDisplay_SetSOC(SOC);
    strncpy(ErrMsg_Evac, DISP_EVAC_REQ_STR_LITERAL, ERR_CODE_LEN);
    Display_Error();
}

#ifndef NODOGS
/**
 * @brief Nested function as the same function needs to be executed however the timer requires different parameters
 * @param p_tmr pointer to the timer that calls this function, passed by timer
 * @param p_arg pointer to the argument passed by timer
 */
static void callbackCANWatchdog(void *p_tmr, void *p_arg)
{
    assertReadCarCANError(READCARCAN_ERR_MISSED_MSG);
}
#endif

/**
 * @brief error handler callback for disabling charging,
 * kills contactor and turns off display
 */
static void handler_ReadCarCAN_contactorsDisable(void)
{
    // Kill contactor using a direct write to avoid blocking calls when the scheduler is locked
    //MotorContactor_EmergencyDisable(); also happens in nonrecoverable errors
    display_err_failed_recovery();
}

static bool check_MotorControllerContactor(void){
    // both should be on at the same time
    bool HVContactorState = Contactors_Get(HV_MINUS_CONTACTOR, true) && Contactors_Get(HV_PLUS_CONTACTOR, true);

    bool motorContactorState = Contactors_Get(MOTOR_CONTROLLER_CONTACTOR, true);
    if(!HVContactorState && motorContactorState)
    {
        // if the HV contactors are off and the motor contactor is on
        return false;
    }
    bool motorPrechargeContactorState = Contactors_Get(MOTOR_CONTROLLER_PRECHARGE_BYPASS_CONTACTOR, true);
    if(motorPrechargeContactorState && !motorContactorState)
    {
        // if the motor precharge contactor is on and the motor contactor is off
        return false;
    }
    return true;
}

/**
 * @brief error handler function to display the evac screen if we get a BPS trip message.
 * Callbacks happen after displaying the fault, so this screen won't get overwritten
 */
static void handler_ReadCarCAN_BPSTrip(void)
{
    MotorContactor_EmergencyDisable();
    Status_Leds_Write(BPS_FAULT_LED, ON); // Turn on BPS fault LED
    Status_Leds_Write(DASH_BPS_HAZ_LED, ON); // Turn on Dashboard BPS Fault LED

    // chargeEnable = false;    // Not really necessary but makes inspection less confusing
    //Display_Evac(SOC, SBPV); // Display evacuation screen             /   /////////////////////////
    display_err_failed_recovery();
}

/**
 * @brief error handle Contactor gives a fault
 * Callbacks happen after displaying the fault, so this screen won't get overwritten
 */
static void handler_ReadCarCAN_ActivePrechargeFault(void)
{
    //Display_Evac(SOC, SBPV); // Display evacuation screen /   /   /   /   /   
    display_err_failed_recovery();
}

/**
 * @brief turns on or off the motor contactor depending on igntion and HV Contactors
 */

static void updateMotorControllerContactor(void){
    ignition_state_t ignState = Get_Ignition_State();
    bool motorContactorState = Contactors_Get(MOTOR_CONTROLLER_CONTACTOR, true);
    if(ignState == IGN_MOTOR || ignState == IGN_ARR){
        if(Contactors_Get(HV_MINUS_CONTACTOR, true) && Contactors_Get(HV_PLUS_CONTACTOR, true)){
            // turn on motor contactor if it was off before
            if(motorContactorState == OFF){
                Contactors_Set(MOTOR_CONTROLLER_CONTACTOR, ON, true);
                return;
            }
        }
    }
    // only setting it off if it was on to avoid an extra scheduling point
    if(motorContactorState == ON){
        Contactors_Set(MOTOR_CONTROLLER_CONTACTOR, OFF, true);
    }
}


void Task_ReadCarCAN(void *p_arg)
{
    #ifndef NODOGS
    OS_ERR err;
    #endif

    // data struct for CAN message
    CANDATA_t dataBuf;

    #ifndef NODOGS
    // Create the CAN Watchdog (periodic) timer, which disconnects the array and disables regenerative braking
    // if we do not get a CAN message with the ID BPS_CONTACTOR within the desired interval.
    OSTmrCreate(
        &canWatchTimer,
        "CAN Watch Timer",
        CAN_WATCH_TMR_DLY_TMR_TS, // Initial delay equal to the period since 0 doesn't seem to work
        CAN_WATCH_TMR_DLY_TMR_TS,
        OS_OPT_TMR_PERIODIC,
        callbackCANWatchdog,
        NULL,
        &err);
    assertOSError(err);

    // Start CAN Watchdog timer
    OSTmrStart(&canWatchTimer, &err);
    assertOSError(err);

    // Create the Active Precharge CAN Watchdog (periodic) timer, which disconnects the array and disables regenerative braking
    // if we do not get a CAN message with the ID CONTACTOR_SENSE within the desired interval.
    OSTmrCreate(
        &prechargeCanWatchTimer,
        "Active Precharge CAN Watch Timer",
        CAN_WATCH_TMR_DLY_TMR_TS, // Initial delay equal to the period since 0 doesn't seem to work
        CAN_WATCH_TMR_DLY_TMR_TS,
        OS_OPT_TMR_PERIODIC,
        callbackCANWatchdog,
        NULL,
        &err);
    assertOSError(err);

    // Start Precharge CAN Watchdog timer
    OSTmrStart(&prechargeCanWatchTimer, &err);
    assertOSError(err);
    #endif

    while (1)
    {

        ErrorStatus status = CANbus_Read(&dataBuf, true, CARCAN);
        #ifdef TASK_PROFILER
        DebugIO_Toggle(READ_CARCAN_PIN);
        #endif
        if (status != SUCCESS)
        {
            continue;
        }
        updateMotorControllerContactor(); // Update motor contactor state based on ignition and HV contactors
        switch (dataBuf.ID)
        {
        case BPS_TRIP:
        { 
            // BPS has a fault and we need to enter fault state
            if(dataBuf.data[0] == BPS_TRIP_MESSAGE)
            {
                // kill motor contactor and enter a nonrecoverable fault
                assertReadCarCANError(READCARCAN_ERR_BPS_TRIP);            
            }
            break;
        }
        case BPS_CONTACTOR:
        {
            #ifndef NODOGS
            OSTmrStart(&canWatchTimer, &err); // Restart CAN Watchdog timer for BPS Contactor msg
            assertOSError(err);
            #endif

            // Set HV+, HV-, and Array Contactor states
            // Note, does not control the Contactors, only stores the received state
            Contactors_Set(HV_PLUS_CONTACTOR, (bool)(dataBuf.data[0] & HV_PLUS_CONTACTOR_BIT), true);
            Contactors_Set(HV_MINUS_CONTACTOR, (bool)(dataBuf.data[0] & HV_MINUS_CONTACTOR_BIT), true);
            Contactors_Set(ARRAY_CONTACTOR, (bool)(dataBuf.data[0] & HV_ARRAY_CONTACTOR_BIT), true);
            break; // End of BPS Contactor Status Updates
        }

        case SUPPLEMENTAL_VOLTAGE:
        {
            SBPV = *(uint16_t *)dataBuf.data;
            UpdateDisplay_SetSBPV(SBPV); // Receive value in mV
            break;
        }
        case STATE_OF_CHARGE:
        {
            SOC = (*(uint32_t *)dataBuf.data) / (SOC_SCALER); // Convert to integer percent
            UpdateDisplay_SetSOC(SOC);
            break;
        }
        case VOLTAGE_SUMMARY:
        {
            UpdateDisplay_SetBattVoltage((*((uint32_t *)dataBuf.data)) & ~0xFF000000); // uint24_t
            break;
        }
        case TEMPERATURE_SUMMARY:
        {
            UpdateDisplay_SetBattTemperature((*((int32_t *)dataBuf.data)) & ~0xFF000000); // uint24_t
            break;
        }
        case CURRENT_DATA:
        {
            UpdateDisplay_SetBattCurrent((*(int32_t *)dataBuf.data)); // int32_t
            break;
        }
        case CONTACTOR_SENSE:
        {
            #ifndef NODOGS
            OSTmrStart(&prechargeCanWatchTimer, &err); // Restart CAN Watchdog timer for Active Precharge Contactor msg
            assertOSError(err);
            #endif

            // Update Motor Contactor sense state
            Contactors_Set(MOTOR_CONTROLLER_CONTACTOR, MOTOR_SENSE_ACTUAL_VALUE(dataBuf.data), true);
            // Update Array Precharge sense state
            Contactors_Set(ARRAY_PRECHARGE_BYPASS_CONTACTOR, ARRAY_PRECHARGE_ACTUAL_VALUE(dataBuf.data), true);
            // Update Motor Precharge sense state
            Contactors_Set(MOTOR_CONTROLLER_PRECHARGE_BYPASS_CONTACTOR, MOTOR_PRECHARGE_ACTUAL_VALUE(dataBuf.data), true);
        
            Status_Leds_Write(MOTOR_PRECHARGE_CONTACTOR_LED, Contactors_Get(MOTOR_CONTROLLER_PRECHARGE_BYPASS_CONTACTOR, true));
            Status_Leds_Write(ARRAY_PRECHARGE_CONTACTOR_LED, Contactors_Get(ARRAY_PRECHARGE_BYPASS_CONTACTOR, true));

            // check to see if motor controller contactor is in expected state
            if(!check_MotorControllerContactor()){
                assertReadCarCANError(READCARCAN_ERR_ACTIVE_PRECHARGE_FAULT);
            }

            // Contactor driver indicates a sense fault
            if(MOTOR_SENSE_FAULT(dataBuf.data) || MOTOR_PRECHARGE_SENSE_FAULT(dataBuf.data) || ARRAY_PRECHARGE_SENSE_FAULT(dataBuf.data))
            {
                assertReadCarCANError(READCARCAN_ERR_ACTIVE_PRECHARGE_FAULT);
            }                        
            break;
        }

        default:
        {
            break; // Unhandled CAN message IDs, do nothing
        }
        }
        #ifdef TASK_PROFILER
        DebugIO_Toggle(READ_CARCAN_PIN);
        #endif
    }
}

/**
 * @brief error assertion function for ReadCarCAN, used to disable charging and handle BPS trip messages
 * Stores the error code and calls assertTaskError with the appropriate parameters and callback handler
 * @param  rcc_err error code to specify the issue encountered
 */
void assertReadCarCANError(ReadCarCAN_error_code_t rcc_err)
{
    Error_ReadCarCAN = (error_code_t)rcc_err; // Store error code for inspection
    set_errmsg_hex("RCC", ErrMsg_ReadCarCAN, rcc_err);    // Store error message for inspection
    
    switch (rcc_err)
    {
    case READCARCAN_ERR_NONE:
        break;

    case READCARCAN_ERR_MISSED_MSG: // Missed message- turn off array and motor controller PBC
        throwTaskError(Error_ReadCarCAN, handler_ReadCarCAN_contactorsDisable, OPT_LOCK_SCHED, OPT_NONRECOV);
        break;

    case READCARCAN_ERR_BPS_TRIP: // Received a BPS trip msg (0 or 1), need to shut down car and infinite loop
        throwTaskError(Error_ReadCarCAN, handler_ReadCarCAN_BPSTrip, OPT_LOCK_SCHED, OPT_NONRECOV);
        break;
    case READCARCAN_ERR_ACTIVE_PRECHARGE_FAULT:
        throwTaskError(Error_ReadCarCAN, handler_ReadCarCAN_ActivePrechargeFault, OPT_LOCK_SCHED, OPT_NONRECOV);
        break;

    default:
        break;
    }

    Error_ReadCarCAN = READCARCAN_ERR_NONE; // Clear the error after handling it
}