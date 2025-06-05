/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file ReadCarCAN.c
 * @brief
 *
 */

#include "os_cfg_app.h"

#include "CANbus.h"
#include "Contactors.h"
#include "DebugIO.h"
#include "Ignition.h"
#include "StatusLeds.h"

#include "ReadCarCAN.h"
#include "Tasks.h"
#include "UpdateDisplay.h"
#include "daybreak_pins.h"

// #define BPS_CAN_WATCHDOG
// #define PRECHARGE_CAN_WATCHDOG

#define MOTOR_PRECHARGE_ON_COUNT_THRESHOLD 5

// Timer delay constants
#define CAN_WATCH_TMR_DLY_MS               1000u // 500 ms
#define CAN_WATCH_TMR_DLY_TMR_TS           ((CAN_WATCH_TMR_DLY_MS * OS_CFG_TMR_TASK_RATE_HZ) / (1000u))

// High Voltage BPS Contactor bit mapping
#define HV_ARRAY_CONTACTOR_BIT             1 << 0 // 0b001
#define HV_MINUS_CONTACTOR_BIT             1 << 1 // 0b010
#define HV_PLUS_CONTACTOR_BIT              1 << 2 // 0b100

// State of Charge scalar to scale it to correct fixed point
#define SOC_SCALER                         1000000

// BPS CAN watchdog timer variable
static OS_TMR canWatchTimer;

// Active Precharge CAN watchdog timer variable
static OS_TMR prechargeCanWatchTimer;

// State of Charge (SOC) and supplemental battery pack voltage (SBPV) value intialization
static uint32_t SOC = 0;
static uint32_t SBPV = 0;

// Boolean to indicate internally whether bps has been checked (for Daybreak => HV+ & HV- have
// already been turned on to avoid extra CAN messages) NOTE: For nextgen, BPS status (contactors,
// checked, trip, etc) will be done more compactly in a packed BPS message
static bool bps_checked = false;

// Error assertion function prototype
// static void assertReadCarCANError(ReadCarCAN_error_code_t rcc_err);

/**
 * @brief Nested function as the same function needs to be executed however the timer requires
 * different parameters
 * @param p_tmr pointer to the timer that calls this function, passed by timer
 * @param p_arg pointer to the argument passed by timer
 */
static void callbackCANWatchdog(void *p_tmr, void *p_arg) {
    // BPS CAN Timer called
    if (p_tmr == &canWatchTimer) {
        assertReadCarCANError(C_ERR_RCC_BPS_MISSED_MSG);
    }
    // Precharge CAN Timer called
    else {
        assertReadCarCANError(C_ERR_RCC_PRECHARGE_MISSED_MSG);
    }
}

// IS THIS ACTUALLY UNUSED? OR NAH?
// static bool check_MotorControllerContactor(void) {
//     // both should be on at the same time
//     bool HVContactorState =
//         Contactors_Get(HV_MINUS_CONTACTOR, true) && Contactors_Get(HV_PLUS_CONTACTOR, true);

//     bool motorContactorState = Contactors_Get(MOTOR_CONTROLLER_CONTACTOR, true);
//     if (!HVContactorState && motorContactorState) {
//         // if the HV contactors are off and the motor contactor is on
//         return false;
//     }
//     bool motorPrechargeContactorState =
//         Contactors_Get(MOTOR_CONTROLLER_PRECHARGE_BYPASS_CONTACTOR, true);
//     if (motorPrechargeContactorState && !motorContactorState) {
//         // if the motor precharge contactor is on and the motor contactor is off
//         return false;
//     }
//     return true;
// }

/**
 * @brief error handler function to display the evac screen if we get a BPS trip message.
 * Callbacks happen after displaying the fault, so this screen won't get overwritten
 */
static void handler_ReadCarCAN_BPSTrip(void) {
    // OS_ERR err;
    MotorContactor_EmergencyDisable();
    Status_Leds_Write(BPS_FAULT_LED, ON);    // Turn on BPS fault LED
    Status_Leds_Write(DASH_BPS_HAZ_LED, ON); // Turn on Dashboard BPS Fault LED
    // OSFlagPost(&BPS_Motor_Status_Flags, BPS_SAFE | MOTOR_SAFE_TO_RUN, OS_OPT_POST_FLAG_CLR, &err); 
    // assertOSError(err);
}

static void setMotorControllerContactor(bool state, bool blocking) {
    Contactors_Set(MOTOR_CONTROLLER_CONTACTOR, state, blocking);

    // If the motor contactor is turned off, we should not be running the motor controller
    if (state == OFF) {
        // OS_ERR err;
        // OSFlagPost(&BPS_Motor_Status_Flags, MOTOR_SAFE_TO_RUN, OS_OPT_POST_FLAG_SET, &err);
        MotorStatus_ModifyBits(MOTOR_SAFE_TO_RUN, true, !OS_FLAG_SCHED_POINT);
    }
    // Turning the motor controller does not necessarily imply the motor is safe to run due to waiting for precharge
}

/**
 * @brief turns on or off the motor contactor depending on igntion and HV Contactors
 */

// static void updateMotorControllerContactor(void){
//     ignition_state_t ignState = Get_Ignition_State();
//     bool motorContactorState = Contactors_Get(MOTOR_CONTROLLER_CONTACTOR, true);
//     if (ignState == IGN_ERROR || ignState == IGN_TRANSITION) {return;}
//     if(ignState == IGN_MOTOR || ignState == IGN_ARR){
//         if(Contactors_Get(HV_MINUS_CONTACTOR, true) && Contactors_Get(HV_PLUS_CONTACTOR, true)){
//             // turn on motor contactor if it was off before
//             if(motorContactorState == OFF){
//                 setMotorControllerContactor(ON, true);
//                 return;
//             }
//         }
//   }
//   if(Contactors_Get(MOTOR_CONTROLLER_CONTACTOR, false) == ON){
//     setMotorControllerContactor(OFF, true); // turn off motor contactor if it was on before
//   }
// }

void Task_ReadCarCAN(void *p_arg) {
    OS_ERR err;

    // data struct for CAN message
    CANDATA_t dataBuf = {0};

    // Create the CAN Watchdog (periodic) timer, which disconnects the array and disables
    // regenerative braking if we do not get a CAN message with the ID BPS_CONTACTOR within the
    // desired interval.
    OSTmrCreate(
        &canWatchTimer, "CAN Watch Timer",
        CAN_WATCH_TMR_DLY_TMR_TS, // Initial delay equal to the period since 0 doesn't seem to work
        CAN_WATCH_TMR_DLY_TMR_TS, OS_OPT_TMR_PERIODIC, callbackCANWatchdog, NULL, &err);
    assertOSError(err);

#ifdef BPS_CAN_WATCHDOG
    // Start CAN Watchdog timer
    OSTmrStart(&canWatchTimer, &err);
    assertOSError(err);
#endif

    OSTmrCreate(
        &prechargeCanWatchTimer, "Active Precharge CAN Watch Timer",
        CAN_WATCH_TMR_DLY_TMR_TS, // Initial delay equal to the period since 0 doesn't seem to work
        CAN_WATCH_TMR_DLY_TMR_TS, OS_OPT_TMR_PERIODIC, callbackCANWatchdog, NULL, &err);
    assertOSError(err);
    // Start Precharge CAN Watchdog timer

#ifdef PRECHARGE_CAN_WATCHDOG
    OSTmrStart(&prechargeCanWatchTimer, &err);
    assertOSError(err);
#endif

    while (1) {
        ErrorStatus status = CANbus_Read(&dataBuf, true, CARCAN);

#ifdef TASK_PROFILER
        DebugIO_Toggle(READ_CARCAN_PIN);
#endif
        if (status != SUCCESS) {
            continue;
        }

        // updateMotorControllerContactor(); // Update motor contactor state based on ignition and
        // HV contactors
        switch (dataBuf.ID) {
            case BPS_TRIP: {
                // BPS has a fault and we need to enter fault state
                if (dataBuf.data[0] == BPS_TRIP_MESSAGE) {
                    // kill motor contactor and enter a nonrecoverable fault
                    assertReadCarCANError(C_ERR_RCC_BPS_TRIP);
                }
                break;
            }
            case BPS_CONTACTOR: {
#ifdef BPS_CAN_WATCHDOG
                OSTmrStart(&canWatchTimer, &err);
                assertOSError(err);
#endif

                // Set HV+, HV-, and Array Contactor states
                // Note, does not control the Contactors, only stores the received state
                Contactors_Set(HV_PLUS_CONTACTOR, (bool)(dataBuf.data[0] & HV_PLUS_CONTACTOR_BIT),
                               true);
                Contactors_Set(HV_MINUS_CONTACTOR, (bool)(dataBuf.data[0] & HV_MINUS_CONTACTOR_BIT),
                               true);
                Contactors_Set(ARRAY_CONTACTOR, (bool)(dataBuf.data[0] & HV_ARRAY_CONTACTOR_BIT),
                               true);
                uint8_t bps_state = (Contactors_Get(HV_MINUS_CONTACTOR, false) &&
                                     Contactors_Get(HV_PLUS_CONTACTOR, false));

                // Context switches should not occur here since the only task pending on these flags
                // waits for all bits to be set Mark BPS checked if its the first time
                if(!bps_checked) {
                    MotorStatus_ModifyBits(BPS_CHECKED, true, !OS_FLAG_SCHED_POINT);
                    // OSFlagPost(&BPS_Motor_Status_Flags, BPS_CHECKED, OS_OPT_POST_FLAG_SET, &err);
                    // assertOSError(err);
                    bps_checked = true;
                }

                // HV contactor used to determine BPS safety
                if (bps_state) {
                    MotorStatus_ModifyBits(BPS_SAFE, true, !OS_FLAG_SCHED_POINT);
                    // OSFlagPost(&BPS_Motor_Status_Flags, BPS_SAFE, OS_OPT_POST_FLAG_SET, &err);
                }
                else {
                    MotorStatus_ModifyBits(BPS_SAFE | MOTOR_SAFE_TO_RUN, false, !OS_FLAG_SCHED_POINT);
                    // OSFlagPost(&BPS_Motor_Status_Flags, BPS_SAFE | MOTOR_SAFE_TO_RUN, OS_OPT_POST_FLAG_CLR, &err);
                }
                assertOSError(err);
                break; // End of BPS Contactor Status Updates
            }

            case SUPPLEMENTAL_VOLTAGE: {
                SBPV = *(uint16_t *)dataBuf.data;
                UpdateDisplay_SetSBPV(SBPV); // Receive value in mV
                break;
            }
            case STATE_OF_CHARGE: {
                SOC = (*(uint32_t *)dataBuf.data) / (SOC_SCALER); // Convert to integer percent
                UpdateDisplay_SetSOC(SOC);
                break;
            }
            case VOLTAGE_SUMMARY: { // uint24_t
                UpdateDisplay_SetBattVoltage((*((uint32_t *)dataBuf.data)) & ~0xFF000000);
                break;
            }
            case PRECHARGE_TIMEOUT: {
                if (dataBuf.data[0] & 0x01) {
                    assertReadCarCANError(C_ERR_RCC_PRECHARGE_TMOUT_MOT);
                } else {
                    assertReadCarCANError(C_ERR_RCC_PRECHARGE_TMOUT_ARR);
                }
                break;
            }
            case CONTACTOR_SENSE: {
                // counter to ensure motor precharge stays on for a few iterations
                static volatile uint8_t motorPrechargeOnCount = 0;
#ifdef PRECHARGE_CAN_WATCHDOG
                // Restart CAN Watchdog timer for Active Precharge Contactor msg
                OSTmrStart(&prechargeCanWatchTimer, &err);
                assertOSError(err);
#endif

                // More things involved with setting the motor controller contactor, so use this
                // function instead
                setMotorControllerContactor(MOTOR_SENSE_ACTUAL_VALUE(dataBuf.data), true);

                Status_Leds_Write(CONTROLS_FAULT, true);
                // Update Array Precharge sense state
                Contactors_Set(ARRAY_PRECHARGE_BYPASS_CONTACTOR,
                               ARRAY_PRECHARGE_ACTUAL_VALUE(dataBuf.data), true);
                // Update Motor Precharge sense state
                Contactors_Set(MOTOR_CONTROLLER_PRECHARGE_BYPASS_CONTACTOR,
                               MOTOR_PRECHARGE_ACTUAL_VALUE(dataBuf.data), true);

                if (Contactors_Get(MOTOR_CONTROLLER_CONTACTOR, true) &&
                    Contactors_Get(MOTOR_CONTROLLER_PRECHARGE_BYPASS_CONTACTOR, true)) {
                    motorPrechargeOnCount++;
                    if (motorPrechargeOnCount >= MOTOR_PRECHARGE_ON_COUNT_THRESHOLD) {
                        // If the motor precharge contactor has been on for enough iterations,
                        // we can consider it safe to run
                        MotorStatus_ModifyBits(MOTOR_SAFE_TO_RUN, true, !OS_FLAG_SCHED_POINT);
//                         OS_ERR err;
//                         OSFlagPost(&BPS_Motor_Status_Flags, MOTOR_SAFE_TO_RUN, OS_OPT_POST_FLAG_SET,
//                                    &err);
//                         assertOSError(err);
                    }
                }
            }
            case TEMPERATURE_SUMMARY: { // uint24_t
                UpdateDisplay_SetBattTemperature((*((int32_t *)dataBuf.data)) & ~0xFF000000);
                break;
            }
            case CURRENT_DATA: { // int32_t
                UpdateDisplay_SetBattCurrent((*(int32_t *)dataBuf.data));
                break;
            }

            default: {
                break; // Unhandled CAN message IDs, do nothing
            }
        }
#ifdef TASK_PROFILER
        DebugIO_Toggle(READ_CARCAN_PIN);
#endif
    }
}

/**
 * @brief error assertion function for ReadCarCAN, used to disable charging and handle BPS
 * trip messages Stores the error code and calls assertTaskError with the appropriate
 * parameters and callback handler
 * @param  rcc_err error code to specify the issue encountered
 */
void assertReadCarCANError(controls_error_e rcc_err) {
    UpdateDisplay_SetSBPV(SBPV);
    UpdateDisplay_SetSOC(SOC);

    switch (rcc_err) {
        case C_ERR_NONE:
            break;

        case C_ERR_RCC_GENERIC:
        case C_ERR_RCC_BPS_MISSED_MSG:
        case C_ERR_RCC_PRECHARGE_MISSED_MSG:
        case C_ERR_RCC_ACTIVE_PRECHARGE_FLT:
        case C_ERR_RCC_PRECHARGE_TMOUT_MOT:
        case C_ERR_RCC_PRECHARGE_TMOUT_ARR:
            throwTaskError(rcc_err, true, NULL, OPT_LOCK_SCHED, OPT_NONRECOV);
            break;

        case C_ERR_RCC_BPS_TRIP:
            throwTaskError(rcc_err, true, handler_ReadCarCAN_BPSTrip, OPT_LOCK_SCHED, OPT_NONRECOV);
            break;

        default:
            // Critical failure, we have a non readcarcan error in readcarcan somehow
            throwTaskError(C_ERR_ILLEGAL_ERROR, false, NULL, OPT_LOCK_SCHED, OPT_NONRECOV);
            break;
    }
}