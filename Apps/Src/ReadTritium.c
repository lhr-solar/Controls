/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file ReadTritium.c
 * @brief
 *
 */

#include "ReadTritium.h"

#include <string.h>

#include "CanBus.h"
#include "SendCarCan.h"
#include "UpdateDisplay.h"
#include "os_cfg_app.h"

// status limit flag masks
#define MASK_MOTOR_TEMP_LIMIT \
    (1 << 6)  // check if motor temperature is limiting the motor
#define MAX_CAN_LEN 8
#define RESTART_THRESHOLD \
    3  // Number of times to restart before asserting a nonrecoverable error
#define MOTOR_TIMEOUT_SECS 1  // Timeout for several missed motor messages
#define MOTOR_TIMEOUT_TICKS (MOTOR_TIMEOUT_SECS * OS_CFG_TMR_TASK_RATE_HZ)

TritiumErrorCode motor_fault_bitmap =
    kNone;  // initialized to no error, changed when the motor asserts an error
static float motor_rpm = 0;
static float motor_velocity = 0;

static OS_TMR motor_watchdog;

// Function prototypes
static void assertTritiumError(TritiumErrorCode motor_err);

// Callback for motor watchdog
static void motorWatchdog(void *tmr, void *p_arg) {
    // Attempt to restart 3 times, then fail
    assertTritiumError(kMotorWatchdogTrip);
}

void TaskReadTritium(void *p_arg) {
    OS_ERR err = 0;
    CanData data_buf = {0};

    // Timer doesn't seem to trigger without initial delay? Might be an RTOS bug
    static bool watchdog_created = false;

    while (1) {
        ErrorStatus status = CanBusRead(&data_buf, true, MOTORCAN);

        if (status == SUCCESS) {
            if (!watchdog_created) {
                OSTmrCreate(&motor_watchdog, "Motor watchdog",
                            MOTOR_TIMEOUT_TICKS, MOTOR_TIMEOUT_TICKS,
                            OS_OPT_TMR_PERIODIC, motorWatchdog, NULL, &err);
                ASSERT_OS_ERROR(err);

                OSTmrStart(&motor_watchdog, &err);
                ASSERT_OS_ERROR(err);

                watchdog_created = true;
            }

            switch (data_buf.id) {
                case kMotorStatus: {
                    // motor status error flags is in bytes 4-5
                    motor_fault_bitmap = *(
                        (uint16_t *)(&data_buf
                                          .data[4]));  // Storing error flags
                                                       // into Motor_FaultBitmap
                    assertTritiumError(motor_fault_bitmap);
                    break;
                }

                case kVelocity: {
                    OSTmrStart(&motor_watchdog, &err);  // Reset the watchdog
                    ASSERT_OS_ERROR(err);
                    memcpy(&motor_rpm, &data_buf.data[0], sizeof(float));
                    memcpy(&motor_velocity, &data_buf.data[4], sizeof(float));

                    // Motor RPM is in bytes 0-3
                    motor_rpm = *((float *)(&data_buf.data[0]));

                    // Car kVelocity (in m/s) is in bytes 4-7
                    motor_velocity = *((float *)(&data_buf.data[4]));
                    uint32_t car_velocity = (uint32_t)motor_velocity;

                    car_velocity =
                        ((car_velocity * 100) *
                         3600);  // Converting from m/s to m/h, // NOLINT
                                 // using fixed point factor of 100
                    car_velocity = ((car_velocity / 160934) *  // NOLINT
                                    10);  // NOLINT // Converting from m/h to
                                          // mph, multiplying by 10 to make
                                          // value "larger" for displaying

                    UpdateDisplaySetVelocity(car_velocity);
                }

                default: {
                    break;  // for cases not handled currently
                }
            }

            SendCarCanPut(data_buf);  // Forward message on CarCAN for telemetry
        }
#ifdef MOCKING
        break;
#endif
    }
}

static void restartMotorController(void) {
    CanData resetmsg = {0};
    resetmsg.id = kMotorReset;
    CanBusSend(resetmsg, true, MOTORCAN);
}

float MotorRpmGet() {  // getter function for motor RPM
    return motor_rpm;
}

float MotorVelocityGet() {  // getter function for motor velocity
    return motor_velocity;
}

/**
 * Error handler functions
 * Passed as callback functions to the main ThrowTaskError function by
 * assertTritiumError
 */

/**
 * @brief A callback function to be run by the main ThrowTaskError function for
 * hall sensor errors restart the motor if the number of hall errors is still
 * less than the MOTOR_RESTART_THRESHOLD.
 */
static inline void handlerReadTritiumHallError(void) {
    restartMotorController();
}

/**
 * @brief   Assert a Tritium error by checking Motor_FaultBitmap
 * and asserting the error with its handler callback if one exists.
 *  Can result in restarting the motor (for hall sensor errors while less than
 * MOTOR_RESTART_THRESHOLD) or locking the scheduler and entering a
 * nonrecoverable fault (all other cases)
 * @param   motor_err Bitmap with motor error codes to check
 */
static void assertTritiumError(TritiumErrorCode motor_err) {
    static uint8_t hall_fault_cnt = 0;  // trip counter, doesn't ever reset
    static uint8_t motor_fault_cnt = 0;

    error_read_tritium =
        (ErrorCode)motor_err;  // Store error codes for inspection info
    if (motor_err == kNone) {
        return;  // No error, return
    }
    if (motor_err != kHallSensorErr && motor_err != kMotorWatchdogTrip) {
        // Assert a nonrecoverable error with no callback function-
        // nonrecoverable will kill the motor and infinite loop
        ThrowTaskError(error_read_tritium, NULL, kOptLockSched, kOptNonrecov);
        return;
    }

    // If it's purely a hall sensor error, try to restart the motor a few times
    // and then fail out

    if (motor_err == kHallSensorErr &&
        ++hall_fault_cnt > RESTART_THRESHOLD) {  // Threshold has been exceeded
        // Assert a nonrecoverable error that will kill the motor, display a
        // fault screen, and infinite loop
        ThrowTaskError(error_read_tritium, NULL, kOptLockSched, kOptNonrecov);
        return;
    }

    // try to restart the motor a few times and then fail out
    if (motor_err == kMotorWatchdogTrip &&
        ++motor_fault_cnt > RESTART_THRESHOLD) {
        // Assert a nonrecoverable error that will kill the motor, display a
        // fault screen, and infinite loop
        ThrowTaskError(error_read_tritium, NULL, kOptLockSched, kOptNonrecov);
        return;
    }

    // Threshold hasn't been exceeded, so assert a recoverable error with the
    // motor restart callback function
    ThrowTaskError(error_read_tritium, handlerReadTritiumHallError,
                   kOptNoLockSched, kOptRecov);

    error_read_tritium = kNone;  // Clear the error after handling it
}
