/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file ReadTritium.c
 * @brief
 *
 */

#include "os_cfg_app.h"

#include "CANbus.h"

#include "ReadTritium.h"
#include "SendCarCAN.h"
#include "Tasks.h"
#include "UpdateDisplay.h"

// status limit flag masks
// #define MASK_MOTOR_TEMP_LIMIT (1 << 6) // check if motor temperature is limiting the motor
// #define MAX_CAN_LEN           8

#define RESTART_THRESHOLD   3 // Number of times to restart before asserting a nonrecoverable error
#define MOTOR_TIMEOUT_SECS  1 // Timeout for several missed motor messages
#define MOTOR_TIMEOUT_TICKS (MOTOR_TIMEOUT_SECS * OS_CFG_TMR_TASK_RATE_HZ)
#define MOTOR_ERROR_MASK    0x01FF

uint16_t Motor_FaultBitmap = 0x0000;
float Motor_RPM = 0;
static float Motor_Velocity = 0;
static float Motor_BusVoltage = 0;
static float Motor_BusCurrent = 0;

#define MPH_CONVERSION      2.236936f // mph = m/s * MPH_CONVERSION

static OS_TMR MotorWatchdog;

// Function prototypes
// static void assertTritiumError(tritium_error_code_t motor_err);

// Callback for motor watchdog
static void motorWatchdog(void *tmr, void *p_arg) {
    // Attempt to restart 3 times, then fail
    assertTritiumError(C_ERR_RTR_MOTOR_WDOG_TRIP);
}

static controls_error_e convert_motorfault_to_error(void) {
    if (Motor_FaultBitmap == 0) return C_ERR_NONE;

    // Check if more than 1 bit is flipped
    if (Motor_FaultBitmap & (Motor_FaultBitmap - 1)) return C_ERR_RTR_MULTIPLE;

    if (Motor_FaultBitmap & (1 << 0)) return C_ERR_RTR_HARDWARE_OC;
    if (Motor_FaultBitmap & (1 << 1)) return C_ERR_RTR_SOFTWARE_OC;
    if (Motor_FaultBitmap & (1 << 2)) return C_ERR_RTR_DC_BUS_OV;
    if (Motor_FaultBitmap & (1 << 3)) return C_ERR_RTR_HALL_SENSOR;
    if (Motor_FaultBitmap & (1 << 4)) return C_ERR_RTR_WDOG_LAST_RESET;
    if (Motor_FaultBitmap & (1 << 5)) return C_ERR_RTR_CONFIG_READ;
    if (Motor_FaultBitmap & (1 << 6)) return C_ERR_RTR_UNDERVOLT_LOCKOUT;
    if (Motor_FaultBitmap & (1 << 7)) return C_ERR_RTR_DESAT_FAULT;
    if (Motor_FaultBitmap & (1 << 8)) return C_ERR_RTR_MOTOR_OVERSPEED;

    return C_ERR_RTR_UNKNOWN_ERROR; // Current error matches no known error
}

void Task_ReadTritium(void *p_arg) {
    OS_ERR err;
    CANDATA_t dataBuf = {0};

    static bool watchdogCreated = false;

    while (1) {
        ErrorStatus status = CANbus_Read(&dataBuf, true, MOTORCAN);
        // An error in the can read but not an os error, signifies that the recv queue is empty

        if (status == SUCCESS) {
            // Timer doesn't seem to trigger without initial delay? Might be an RTOS bug
            if (!watchdogCreated) {
                OSTmrCreate(&MotorWatchdog, "Motor watchdog", MOTOR_TIMEOUT_TICKS,
                            MOTOR_TIMEOUT_TICKS, OS_OPT_TMR_PERIODIC, motorWatchdog, NULL, &err);
                assertOSError(err);

                OSTmrStart(&MotorWatchdog, &err);
                assertOSError(err);

                watchdogCreated = true;
            }

            switch (dataBuf.ID) {
                case MC_BUS: {
                    Motor_BusVoltage = *((float *)&dataBuf.data[0]);
                    Motor_BusCurrent = *((float *)&dataBuf.data[4]);

                    UpdateDisplay_SetMCVoltage(Motor_BusVoltage * 10);
                    UpdateDisplay_SetMCCurrent(Motor_BusCurrent * 10);
                    break;
                }
                
                case MOTOR_STATUS: {
                    // motor status error flags is in bytes 4-5
                    Motor_FaultBitmap = (*((uint16_t *)(&dataBuf.data[4])) & MOTOR_ERROR_MASK);

					// If none of the bits are set, then it will display None
					//UpdateDisplay_SetMotorLimit(*((uint16_t *)(&dataBuf.data[6])));

                    assertTritiumError(convert_motorfault_to_error());
                    break;
                }

                case VELOCITY: {
                    OSTmrStart(&MotorWatchdog, &err); // Reset the watchdog
                    assertOSError(err);
                    memcpy(&Motor_RPM, &dataBuf.data[0], sizeof(float));
                    memcpy(&Motor_Velocity, &dataBuf.data[4], sizeof(float));

                    // Motor RPM is in bytes 0-3
                    Motor_RPM = *((float *)(&dataBuf.data[0]));

                    // Car Velocity (in m/s) is in bytes 4-7
                    Motor_Velocity = *((float *)(&dataBuf.data[4]));

                    float Car_Velocity = Motor_Velocity * MPH_CONVERSION * 10.0f;

                    // Display can't take negative values, and reverse puts Car_Velocity in the negative
                    Car_Velocity = (Car_Velocity < 0) ? -Car_Velocity : Car_Velocity; 

                    // Round car velocity to the nearest integer
                    UpdateDisplay_SetVelocity((uint32_t)(Car_Velocity));

                    break;
                }

                case TEMPERATURE: {
                    UpdateDisplay_SetHeatSinkTemp(*(float *)(&dataBuf.data[4]));
                    break;
                }

                default: {
                    break; // for cases not handled currently
                }
            }

            SendCarCAN_Put(dataBuf); // Forward message on CarCAN for telemetry
        }
    }
}

static void restartMotorController(void) {
    CANDATA_t resetmsg = {0};
    resetmsg.ID = MOTOR_RESET;
    CANbus_Send(resetmsg, true, MOTORCAN);
}

// Getter function for motor RPM
float Motor_RPM_Get() { return Motor_RPM; }

// Getter function for motor velocity
float Motor_Velocity_Get() { return Motor_Velocity; }

// Getter function for motor error
uint16_t Motor_Error_Get() { return Motor_FaultBitmap; }

/**
 * Error handler functions
 * Passed as callback functions to the main throwTaskError function by assertTritiumError
 */

/**
 * @brief A callback function to be run by the main throwTaskError function for hall sensor errors
 * restart the motor if the number of hall errors is still less than the MOTOR_RESTART_THRESHOLD.
 */
static inline void handler_ReadTritium_HallError(void) { restartMotorController(); }

/**
 * @brief   Assert a Tritium error by checking Motor_FaultBitmap
 * and asserting the error with its handler callback if one exists.
 *  Can result in restarting the motor (for hall sensor errors while less than
 * MOTOR_RESTART_THRESHOLD) or locking the scheduler and entering a nonrecoverable fault (all other
 * cases)
 * @param   motor_err Bitmap with motor error codes to check
 */
void assertTritiumError(controls_error_e m_err) {
    static uint8_t hall_fault_cnt = 0; // trip counter, doesn't ever reset
    static uint8_t motor_fault_cnt = 0;

    switch (m_err) {
        case C_ERR_NONE:
            break;

        case C_ERR_RTR_GENERIC:
        case C_ERR_RTR_HARDWARE_OC:
        case C_ERR_RTR_SOFTWARE_OC:
        case C_ERR_RTR_DC_BUS_OV:
        case C_ERR_RTR_WDOG_LAST_RESET:
        case C_ERR_RTR_CONFIG_READ:
        case C_ERR_RTR_UNDERVOLT_LOCKOUT:
        case C_ERR_RTR_DESAT_FAULT:
        case C_ERR_RTR_MOTOR_OVERSPEED:
        case C_ERR_RTR_INIT_FAIL:
        case C_ERR_RTR_MULTIPLE:
        case C_ERR_RTR_UNKNOWN_ERROR:
            throwTaskError(m_err, !EVAC_NEEDED, NULL, OPT_LOCK_SCHED, OPT_NONRECOV, CAN_NONE_BPS);
            break;

        case C_ERR_RTR_MOTOR_WDOG_TRIP:
            // Try to restart the motor a few times and then fail out
            if (++motor_fault_cnt > RESTART_THRESHOLD) {
                throwTaskError(m_err, !EVAC_NEEDED, NULL, OPT_LOCK_SCHED, OPT_NONRECOV, CAN_NONE_BPS);
            } else {
                throwTaskError(m_err, !EVAC_NEEDED, handler_ReadTritium_HallError, OPT_NO_LOCK_SCHED, OPT_NONRECOV, CAN_NONE_BPS);
            }
            break;

        case C_ERR_RTR_HALL_SENSOR:
            // If it's purely a hall sensor error, try to restart the motor a few times and then
            // fail out
            if (++hall_fault_cnt > RESTART_THRESHOLD) {
                throwTaskError(m_err, !EVAC_NEEDED, NULL, OPT_LOCK_SCHED, OPT_NONRECOV, CAN_NONE_BPS);
            } else {
                throwTaskError(m_err, !EVAC_NEEDED, handler_ReadTritium_HallError, OPT_NO_LOCK_SCHED, OPT_NONRECOV, CAN_UNKNOWN_BPS);
            }
            break;

        default:
            // Critical failure, we have a non readtritium error in readtritium somehow
            throwTaskError(C_ERR_ILLEGAL_ERROR, EVAC_NEEDED, NULL, OPT_LOCK_SCHED, OPT_NONRECOV, CAN_NONE_BPS);
            break;
    }
}
