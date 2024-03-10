/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file ReadCarCAN.c
 * @brief
 *
 */

#include "ReadCarCan.h"

#include "CanBus.h"
#include "Contactors.h"
#include "Display.h"
#include "Minions.h"
#include "Tasks.h"
#include "UpdateDisplay.h"
#include "common.h"
#include "os.h"
#include "os_cfg_app.h"

// Length of the array and motor PBC saturation buffers
#define SAT_BUF_LENGTH 5

// The Array/Motor Controller Saturation Threshold is used to determine if
// Controls has
//      received a sufficient number of BPS's HV Array/Plus-Minus Enable
//      Messages. BPS Array and Plus/Minus saturation threshold is halfway
//      between 0 and max saturation value.
#define ARRAY_SATURATION_THRESHOLD (((SAT_BUF_LENGTH + 1) * SAT_BUF_LENGTH) / 4)
#define PLUS_MINUS_SATURATION_THRESHOLD \
    (((SAT_BUF_LENGTH + 1) * SAT_BUF_LENGTH) / 4)

// Timer delay constants
#define CAN_WATCH_TMR_DLY_MS 500u  // 500 ms
#define CAN_WATCH_TMR_DLY_TMR_TS                        \
    ((CAN_WATCH_TMR_DLY_MS * OS_CFG_TMR_TASK_RATE_HZ) / \
     (1000u))  // 1000 for ms -> s conversion

// Precharge Delay times in milliseconds
#define PRECHARGE_PLUS_MINUS_DELAY \
    100u  // 100 ms, as this the smallest time delay that the RTOS can work with
#define PRECHARGE_ARRAY_DELAY 100u  // 100 ms
#define ARRAY_PRECHARGE_BYPASS_DLY_TMR_TS \
    ((PRECHARGE_ARRAY_DELAY * OS_CFG_TMR_TASK_RATE_HZ) / (1000u))
#define MOTOR_CONTROLLER_PRECHARGE_BYPASS_DLY_TMR_TS \
    ((PRECHARGE_PLUS_MINUS_DELAY * OS_CFG_TMR_TASK_RATE_HZ) / (1000u))

// High Voltage BPS Contactor bit mapping
#define HV_ARRAY_CONTACTOR_BIT 1  // 0b001
#define HV_MINUS_CONTACTOR_BIT 2  // 0b010
#define HV_PLUS_CONTACTOR_BIT 4   // 0b100

// Saturation messages
#define DISABLE_SATURATION_MSG (-1)
#define ENABLE_SATURATION_MSG 1

// State of Charge scalar to scale it to correct fixed point
#define SOC_SCALER 1000000

// CAN watchdog timer variable
static OS_TMR can_watch_timer;

// Array precharge bypass contactor delay timer variable
static OS_TMR array_pbc_dly_timer;

// Motor controller precharge bypass contactor delay timer variable
static OS_TMR motor_controller_pbc_dly_timer;

// NOTE: This should not be written to anywhere other than ReadCarCAN. If the
// need arises, a mutex to protect it must be added. Indicates whether or not
// regenerative braking / charging is enabled.
static bool charge_enable =
    false;  // Enable (High message) of BPS high voltage (HV) array contactor

// BPS HV Array saturation buffer variables
static int8_t hv_array_charge_msg_buffer[SAT_BUF_LENGTH];
static int8_t hv_array_msg_saturation = 0;
static uint8_t hv_array_oldest_msg_idx = 0;

// BPS HV Motor Controller saturation buffer variables
static int8_t hv_plus_minus_charge_msg_buffer[SAT_BUF_LENGTH];
static int8_t hv_plus_minus_charge_msg_saturation = 0;
static uint8_t hv_plus_minus_motor_oldest_msg_idx = 0;

// Array ignition (kIgn1) and Motor Controller ignition (kIgn2) pin status
static bool arr_ign_status = false;
static bool mc_ign_status = false;

// Boolean to indicate precharge status for Array Precharge Bypass Contactor
// (PBC) and Motor Controller PBC
static bool arr_pbc_complete = false;
static bool mc_pbc_complete = false;

// State of Charge (SOC) and supplemental battery pack voltage (SBPV) value
// intialization
static uint8_t soc = 0;
static uint32_t sbpv = 0;

// Error assertion function prototype
static void assertReadCarCANError(ReadCarCanErrorCode rcc_err);

// Getter function for charge enable, indicating that battery charging is
// allowed
bool ChargeEnableGet(void) { return charge_enable; }

/**
 * @brief Nested function as the same function needs to be executed however the
 * timer requires different parameters
 * @param p_tmr pointer to the timer that calls this function, passed by timer
 * @param p_arg pointer to the argument passed by timer
 */
static void callbackCANWatchdog(void *p_tmr, void *p_arg) {
    assertReadCarCANError(kReadCarCanErrMissedMsg);
}

/**
 * @brief Callback function for the precharge delay timer. Waits for precharge
 * and then sets arrPBCComplete to true.
 * @param p_tmr pointer to the timer that calls this function, passed by timer
 * @param p_arg pointer to the argument passed by timer
 */
static void setArrayBypassPrechargeComplete(void *p_tmr, void *p_arg) {
    arr_pbc_complete = true;
};

/**
 * @brief Callback function for the precharge delay timer. Waits for precharge
 * and then sets mcPBCComplete to true.
 * @param p_tmr pointer to the timer that calls this function, passed by timer
 * @param p_arg pointer to the argument passed by timer
 */
static void setMotorControllerBypassPrechargeComplete(void *p_tmr,
                                                      void *p_arg) {
    mc_pbc_complete = true;
};

/**
 * @brief Disables Array Precharge Bypass Contactor (PBC) by asserting an error.
 * Also updates display for Array PBC to be open.
 * @param None
 */
static void disableArrayPrechargeBypassContactor(void) {
    // Assert error to disable regen and update saturation in callback function
    assertReadCarCANError(kReadCarCanErrChargeDisable);
    // Turn off the array contactor display light
    UpdateDisplaySetArray(false);  // Can assume contactor turned off or else
                                   // this won't be reached
}

/**
 * @brief Turns array PBC on if conditional meets ignition status, saturation
 * threshold, array PCB to be off, and a non-running precharge delay timer.
 * @param None
 */
static void updateArrayPrechargeBypassContactor(void) {
    OS_ERR err = OS_ERR_NONE;
    if ((arr_ign_status || mc_ign_status)  // Ignition is ON
        && hv_array_msg_saturation >=
               ARRAY_SATURATION_THRESHOLD  // Saturation Threshold has be met
        && (ContactorsGet(kArrayPrechargeBypassContactor) == OFF)
        // Array PBC is OFF
        &&
        (OSTmrStateGet(&array_pbc_dly_timer, &err) !=
         OS_TMR_STATE_RUNNING)) {  // and precharge is currently not happening
        // Asserts error for OS timer state above if conditional was met
        ASSERT_OS_ERROR(err);
        // Wait to make sure precharge is finished and then restart array
        OSTmrStart(&array_pbc_dly_timer, &err);
    }
    // Asserts error for OS timer state above if conditional was not met
    ASSERT_OS_ERROR(err);
}

/**
 * @brief Turns array PBC on if conditional meets ignition status, saturation
 * threshold, motor PBC to be off, and a non-running precharge delay timer.
 * @param None
 */
static void updateMCPBC(void) {
    OS_ERR err = OS_ERR_NONE;
    if (mc_ign_status  // Ignition is ON
        &&
        hv_plus_minus_charge_msg_saturation >=
            PLUS_MINUS_SATURATION_THRESHOLD  // Saturation Threshold has be met
        && (ContactorsGet(kMotorControllerPrechargeBypassContactor) ==
            OFF)  // Motor Controller PBC is OFF
        &&
        (OSTmrStateGet(&motor_controller_pbc_dly_timer, &err) !=
         OS_TMR_STATE_RUNNING)) {  // and precharge is currently not happening
        // Asserts error for OS timer state above if conditional was met
        ASSERT_OS_ERROR(err);
        // Wait to make sure precharge is finished and then restart array
        OSTmrStart(&motor_controller_pbc_dly_timer, &err);
    }
    // Asserts error for OS timer start above if conditional was not met
    ASSERT_OS_ERROR(err);
}

/**
 * @brief adds new messages by overwriting old messages in the saturation buffer
 * and then updates saturation
 * @param messageState whether bps message was enable (1) or disable (-1)
 */
static void updateHVArraySaturation(int8_t message_state) {
    // Replace oldest message with new charge message and update index for
    // oldest message
    hv_array_charge_msg_buffer[hv_array_oldest_msg_idx] = message_state;
    hv_array_oldest_msg_idx = (hv_array_oldest_msg_idx + 1) % SAT_BUF_LENGTH;

    // Calculate the new saturation value by assigning weightings from 1 to
    // buffer length in order of oldest to newest
    int new_saturation = 0;
    for (uint8_t i = 0; i < SAT_BUF_LENGTH; i++) {
        new_saturation +=
            hv_array_charge_msg_buffer[(hv_array_oldest_msg_idx + i) %
                                       SAT_BUF_LENGTH] *
            (i + 1);
    }
    hv_array_msg_saturation = (int8_t)new_saturation;

    if (message_state == -1) {
        charge_enable = false;
    } else if (hv_array_msg_saturation >= ARRAY_SATURATION_THRESHOLD) {
        charge_enable = true;
        updateArrayPrechargeBypassContactor();
    }
}

/**
 * @brief adds new messages by overwriting old messages in the saturation buffer
 * and then updates saturation
 * @param messageState whether bps message was  enable (1) or disable (-1)
 */
static void updateHVPlusMinusSaturation(int8_t message_state) {
    // Replace oldest message with new charge message and update index for
    // oldest message
    hv_plus_minus_charge_msg_buffer[hv_plus_minus_motor_oldest_msg_idx] =
        message_state;
    hv_plus_minus_motor_oldest_msg_idx =
        (hv_plus_minus_motor_oldest_msg_idx + 1) % SAT_BUF_LENGTH;

    // Calculate the new saturation value by assigning weightings from 1 to
    // buffer length in order of oldest to newest
    int new_saturation = 0;
    for (uint8_t i = 0; i < SAT_BUF_LENGTH; i++) {
        new_saturation +=
            hv_plus_minus_charge_msg_buffer
                [(hv_plus_minus_motor_oldest_msg_idx + i) % SAT_BUF_LENGTH] *
            (i + 1);
    }
    hv_plus_minus_charge_msg_saturation = (int8_t)new_saturation;

    if (message_state == 1) {
        updateMCPBC();
    }
}

/**
 * @brief Helper to turn arrayPBCOn if arrayBypassPrecharge is completed and
 * charging is enabled
 * @param None
 */
void AttemptTurnArrayPbcOn(void) {
    if (arr_pbc_complete && charge_enable) {
        ContactorsSet(kArrayPrechargeBypassContactor, ON, true);  // Turn on
        UpdateDisplaySetArray(true);
        arr_pbc_complete = false;
    }
}

/**
 * @brief Helper to turn motorControllerPBCOn if motorControllerBypassPrecharge
 * is completed and threshold is reached
 * @param None
 */
void AttemptTurnMotorControllerPbcOn(void) {
    if (mc_pbc_complete) {
        ContactorsSet(kMotorControllerPrechargeBypassContactor, ON, true);
        UpdateDisplaySetMotor(true);
    }
}

/**
 * @brief Helper to turn motorControllerPBCOff if ignition is not turned to
 * motor or motor controller threshold isn't reached.
 * @param None
 */
void TurnMotorControllerPbcOff(void) {
    ContactorsSet(kMotorControllerPrechargeBypassContactor, OFF, true);
    UpdateDisplaySetMotor(false);
}

/**
 * @brief Turns array and motor controller PBC ON/OFF based on ignition and
 * precharge status
 * @param None
 */
void UpdatePrechargeContactors(void) {
    arr_ign_status = MinionsRead(kIgn1);
    mc_ign_status = MinionsRead(kIgn2);

    // Logic helper in cases both are off or (impossible) on
    bool both_status_off = !mc_ign_status && !arr_ign_status;
    bool both_status_on = mc_ign_status && arr_ign_status;

    if (both_status_off || both_status_on) {
        assertReadCarCANError(
            kReadCarCanErrDisableContactorsMsg);  // Turn Array and Motor
                                                  // Controller PBC off using
                                                  // error assert

    } else if (!mc_ign_status && arr_ign_status) {
        AttemptTurnArrayPbcOn();      // Turn Array PBC On, if permitted
        TurnMotorControllerPbcOff();  // Turn Motor Controller PBC Off

    } else if (mc_ign_status && !arr_ign_status) {
        AttemptTurnArrayPbcOn();  // Turn Array PBC On, if permitted
        if (hv_plus_minus_charge_msg_saturation >=
            PLUS_MINUS_SATURATION_THRESHOLD) {  // Turn Motor Controller PBC On,
                                                // if threshold is reached
            AttemptTurnMotorControllerPbcOn();
        } else {
            TurnMotorControllerPbcOff();
        }
    }

    // Set precharge complete variable to false if precharge happens again
    arr_pbc_complete = false;
    mc_pbc_complete = false;
}

void TaskReadCarCan(void *p_arg) {
    OS_ERR err = 0;

    // data struct for CAN message
    CanData data_buf;

    // Create the CAN Watchdog (periodic) timer, which disconnects the array and
    // disables regenerative braking if we do not get a CAN message with the ID
    // Charge_Enable within the desired interval.
    OSTmrCreate(&can_watch_timer, "CAN Watch Timer",
                CAN_WATCH_TMR_DLY_TMR_TS,  // Initial delay equal to the period
                                           // since 0 doesn't seem to work
                CAN_WATCH_TMR_DLY_TMR_TS, OS_OPT_TMR_PERIODIC,
                callbackCANWatchdog, NULL, &err);
    ASSERT_OS_ERROR(err);

    OSTmrCreate(&array_pbc_dly_timer, "Array Bypass Precharge Delay Timer", 0,
                ARRAY_PRECHARGE_BYPASS_DLY_TMR_TS, OS_OPT_TMR_ONE_SHOT,
                setArrayBypassPrechargeComplete, NULL, &err);
    ASSERT_OS_ERROR(err);

    OSTmrCreate(&motor_controller_pbc_dly_timer,
                "Motor Controller Bypass Precharge Delay Timer", 0,
                MOTOR_CONTROLLER_PRECHARGE_BYPASS_DLY_TMR_TS,
                OS_OPT_TMR_ONE_SHOT, setMotorControllerBypassPrechargeComplete,
                NULL, &err);
    ASSERT_OS_ERROR(err);

    // Start CAN Watchdog timer
    OSTmrStart(&can_watch_timer, &err);
    ASSERT_OS_ERROR(err);

    // Fills buffers with disable messages
    // NOTE: If the buffer becomes bigger than of type int8_t, memset will not
    // work and would need to be reimplemented.
    memset(hv_array_charge_msg_buffer, DISABLE_SATURATION_MSG,
           sizeof(hv_array_charge_msg_buffer));
    memset(hv_plus_minus_charge_msg_buffer, DISABLE_SATURATION_MSG,
           sizeof(hv_plus_minus_charge_msg_buffer));

    while (1) {
        UpdatePrechargeContactors();  // Sets array and motor controller PBC if
                                      // all conditions (PBC Status, Threshold,
                                      // Precharge Complete) permit

        // BPS sent a message
        ErrorStatus status = CanBusRead(&data_buf, true, CARCAN);
        if (status != SUCCESS) {
            continue;
        }

        switch (data_buf.id) {  // Switch case based on BPS msg received
            case kBpsTrip: {    // BPS has a fault and we need to enter fault
                                // state

                // kill contactors and enter a nonrecoverable fault
                assertReadCarCANError(kReadCarCanErrBpsTrip);
            }
            case kBpsContactor: {
                OSTmrStart(
                    &can_watch_timer,
                    &err);  // Restart CAN Watchdog timer for BPS Contactor msg
                ASSERT_OS_ERROR(err);

                // Retrieving HV contactor statuses using bit mapping
                // Bitwise to get HV Plus and Minus, and then &&ing to ensure
                // both are on
                bool hv_plus_minus_status =
                    (bool)((data_buf.data[0] & HV_PLUS_CONTACTOR_BIT) &&
                           (data_buf.data[0] & HV_MINUS_CONTACTOR_BIT));
                // Bitwise to get HV Array
                bool hv_array_status =
                    (bool)(data_buf.data[0] & HV_ARRAY_CONTACTOR_BIT);

                // Update HV Array and HV Plus/Minus saturations based on the
                // respective statuses
                hv_array_status ? updateHVArraySaturation(ENABLE_SATURATION_MSG)
                                : disableArrayPrechargeBypassContactor();
                hv_plus_minus_status
                    ? updateHVPlusMinusSaturation(ENABLE_SATURATION_MSG)
                    : updateHVPlusMinusSaturation(DISABLE_SATURATION_MSG);

                break;  // End of BPS Contactor Status Updates
            }

            case kSupplementalVoltage: {
                sbpv = (*(uint16_t *)&data_buf.data);
                UpdateDisplaySetSbpv(sbpv);  // Receive value in mV
                break;
            }
            case kStateOfCharge: {
                soc = (*(uint32_t *)&data_buf.data) /
                      (SOC_SCALER);  // Convert to integer percent
                UpdateDisplaySetSoc(soc);
                break;
            }
            default: {
                break;  // Unhandled CAN message IDs, do nothing
            }
        }

#ifdef MOCKING
        break;
#endif
    }
}

/**
 * @brief error handler callback for disabling charging,
 * kills contactor and turns off display
 */
static void handlerReadCarCanChargeDisable(void) {
    // Fills buffers with disable messages
    memset(hv_array_charge_msg_buffer, DISABLE_SATURATION_MSG,
           sizeof(hv_array_charge_msg_buffer));

    // mark regen as disabled and update saturation
    updateHVArraySaturation(DISABLE_SATURATION_MSG);

    // Kill contactor using a direct write to avoid blocking calls when the
    // scheduler is locked
    BspGpioWritePin(CONTACTORS_PORT, ARRAY_PRECHARGE_BYPASS_PIN, false);

    // Check that the contactor was successfully turned off
    bool ret = (bool)ContactorsGet(kArrayPrechargeBypassContactor);

    if (ret) {  // Contactor failed to turn off; display the evac screen and
                // infinite loop
        DisplayEvac(soc, sbpv);
#ifndef MOCKING
        while (1) {}
#endif
    }
}

/**
 * @brief error handler callback for disabling charging,
 * kills contactor and turns off display
 */
static void handlerReadCarCanContactorsDisable(void) {
    // Mark regen as disabled and update saturation
    updateHVArraySaturation(DISABLE_SATURATION_MSG);
    updateHVPlusMinusSaturation(DISABLE_SATURATION_MSG);

    // Kill contactor using a direct write to avoid blocking calls when the
    // scheduler is locked
    BspGpioWritePin(CONTACTORS_PORT, ARRAY_PRECHARGE_BYPASS_PIN, false);
    BspGpioWritePin(CONTACTORS_PORT, MOTOR_CONTROLLER_PRECHARGE_BYPASS_PIN,
                    false);

    // Fills buffers with disable messages
    memset(hv_array_charge_msg_buffer, DISABLE_SATURATION_MSG,
           sizeof(hv_array_charge_msg_buffer));
    memset(hv_plus_minus_charge_msg_buffer, DISABLE_SATURATION_MSG,
           sizeof(hv_plus_minus_charge_msg_buffer));

    // Updates the saturation with disable
    updateHVArraySaturation(DISABLE_SATURATION_MSG);
    updateHVPlusMinusSaturation(DISABLE_SATURATION_MSG);

    // Check that the contactor was successfully turned off
    bool ret = (bool)ContactorsGet(kArrayPrechargeBypassContactor) ||
               (bool)ContactorsGet(kMotorControllerPrechargeBypassContactor);

    if (ret) {  // Contactor failed to turn off; display the evac screen and
                // infinite loop
        DisplayEvac(soc, sbpv);
#ifndef MOCKING
        while (1) {}
#endif
    }
}

/**
 * @brief error handler function to display the evac screen if we get a BPS trip
 * message. Callbacks happen after displaying the fault, so this screen won't
 * get overwritten
 */
static void handlerReadCarCanBpsTrip(void) {
    charge_enable =
        false;  // Not really necessary but makes inspection less confusing
    DisplayEvac(soc, sbpv);  // Display evacuation screen
}

/**
 * @brief error assertion function for ReadCarCAN, used to disable charging and
 * handle BPS trip messages Stores the error code and calls assertTaskError with
 * the appropriate parameters and callback handler
 * @param  rcc_err error code to specify the issue encountered
 */
static void assertReadCarCANError(ReadCarCanErrorCode rcc_err) {
    error_read_car_can = (ErrorCode)rcc_err;  // Store error code for inspection

    switch (rcc_err) {
        case kReadCarCanErrNone:
            break;

        case kReadCarCanErrChargeDisable:  // Received a charge disable msg and
                                           // need to turn off array contactor
            ThrowTaskError(error_read_car_can, handlerReadCarCanChargeDisable,
                           kOptLockSched, kOptRecov);
            break;

        case kReadCarCanErrDisableContactorsMsg:
        case kReadCarCanErrMissedMsg:  // Missed message- turn off array and
                                       // motor controller PBC Ignition turned
                                       // to off or turned to last two
                                       // simultaneously
            ThrowTaskError(error_read_car_can,
                           handlerReadCarCanContactorsDisable, kOptLockSched,
                           kOptRecov);
            break;

        case kReadCarCanErrBpsTrip:  // Received a BPS trip msg (0 or 1), need
                                     // to shut down car and infinite loop
            ThrowTaskError(error_read_car_can, handlerReadCarCanBpsTrip,
                           kOptLockSched, kOptRecov);
            break;

        default:
            break;
    }

    error_read_car_can =
        kReadCarCanErrNone;  // Clear the error after handling it
}