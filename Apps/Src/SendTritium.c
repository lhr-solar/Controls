/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file SendTritium.c
 * @brief Function implementations for the SendTritium application minimum viable
 * product with cruise control abilities.
 *
 * This contains functions relevant to updating the velocity and current setpoints
 * of the Tritium motor controller. The implementation includes a normal current
 * controlled mode and a cruise control mode, but doesn't include a one pedal mode/regen
 * braking capabilities. The logic is determined through a finite state machine
 * implementation.
 *
 * If the macro SENDTRITIUM_EXPOSE_VARS is defined prior to including
 * SendTritium.h, relevant setters will be exposed as externs for unit testing
 * and hardware inputs won't be read and motor commands won't be sent over MotorCAN.
 * If the macro SENDTRITIUM_PRINT_MES is also defined prior to including
 * SendTritium.h, debug info will be printed via UART.
 */

#include "os_cfg_app.h"

#include "CANConfig.h"
#include "CANbus.h"
#include "Dashboard.h"
#include "Pedals.h"
#include "StatusLeds.h"

#include "ReadTritium.h"
#include "SendTritium.h"
#include "SendCarCAN.h"
#include "Tasks.h"
#include "UpdateDisplay.h"

// Inputs
static uint8_t brakePedalPercent = 0;
static uint8_t accelPedalPercent = 0;
static gear_t  gear              = DASH_NEU;
static bool    isBrakeOn         = false; // Used for updating display & brakelight

// Outputs
static float currentSetpoint     = 0.0f;
static float velocitySetpoint    = 0.0f;
static float busCurrentSetPoint  = CONT_MOCO_BATTERY_CURRENT / MAX_MOCO_CURRENT;

// NOTE: Instead of a "velocityObserved" variable, we can just use Motor_Velocity_Get() from
// ReadTritium when doing cruise logic

// Getter functions for local variables in SendTritium.c
GETTER(uint8_t, brakePedalPercent)
GETTER(uint8_t, accelPedalPercent)
GETTER(gear_t, gear)
GETTER(float, currentSetpoint)
GETTER(float, velocitySetpoint)
GETTER(bool, isBrakeOn)

// Gear fault counter
static uint8_t gearFaultCnt = 0;

// Function prototypes
static void assertSendTritiumError(controls_error_e sterr);

// Helper Functions

/**
 * @brief Update the accel, brake, & gear on the display +
 * write to the brakelight
 */
static void updateDisplayState() {
    UpdateDisplay_SetAccel(accelPedalPercent);
    UpdateDisplay_SetBrake(isBrakeOn);

    switch (gear) {
        case DASH_FWD:
            UpdateDisplay_SetGear(DISP_FORWARD);
            break;
        case DASH_NEU:
            UpdateDisplay_SetGear(DISP_NEUTRAL);
            break;
        case DASH_REV:
            UpdateDisplay_SetGear(DISP_REVERSE);
            break;
        default:
            UpdateDisplay_SetGear(DISP_NEUTRAL);
            break;
    }

    UpdateDisplay_SetRegenState(DISP_DISABLED);  // Not on Daybreak
    UpdateDisplay_SetCruiseState(DISP_DISABLED); // Probably not on Daybreak
    UpdateDisplay_SetAccel(accelPedalPercent);
}

/**
 * @brief Reads inputs from the system
 */
static void readInputs() {
    brakePedalPercent = Pedals_Read(BRAKE);
    accelPedalPercent = Pedals_Read(ACCELERATOR);

    // Brake hysteresis
    if (brakePedalPercent <= BRAKE_UNPRESSED_THRESHOLD)
        isBrakeOn = false;
    else if (brakePedalPercent >= BRAKE_PRESSED_THRESHOLD)
        isBrakeOn = true;
    Status_Leds_Write(BRAKELIGHT_LED, isBrakeOn); // Write to the dashboard brake light

    gear = getGear();

    // Check for gear fault
    if (gear == DASH_GEAR_FAULT_ERROR) {
        // Fault behavior
        if (gearFaultCnt > GEAR_FAULT_THRESHOLD)
            assertSendTritiumError(C_ERR_STR_GEAR_FAULT);
        else
            gearFaultCnt++;
    } else {
        gearFaultCnt = 0;
    }
}

/**
 * @brief Linearly map range of integers to another range of integers, and provide the pecentage
 * result. in_min to in_max is mapped to out_min to out_max.
 * @param input input integer value
 * @param in_min minimum value of input range
 * @param in_max maximum value of input range
 * @param out_min minimum value of output range
 * @param out_max maximum value of output range
 * @returns float value from (out_min / 100.0) to (out_max / 100.0)
 */
float mapToPercent(uint8_t input, uint8_t in_min, uint8_t in_max, uint8_t out_min,
                   uint8_t out_max) {
    // The minimum of the input range should never be greater than the maximum of the input range
    if (in_min >= in_max) {
        in_max = in_min;
    }

    // Lower bound the input to the minimum possible output
    if (input <= in_min) {
        return out_min / 100.0;
    } else if (input >= in_max) {
        // Upper bound the input to the maximum output
        return out_max / 100.0;
    } else {
        // Linear mapping between ranges
        uint8_t offset_in  = input - in_min; // If input went from A -> B, it now goes from 0 -> B-A
        uint8_t in_range   = in_max - in_min;   // Input range
        uint8_t out_range  = out_max - out_min; // Output range
        uint8_t offset_out = out_min;
        // slope = out_range/in_range. y=mx+b so output=slope*offset_in+offset_out
        return ((offset_in * out_range) / in_range + offset_out) / 100.0;
    }
}

// Task (main loop)

/**
 * @brief Follows the FSM to update the velocity of the car
 */
void Task_SendTritium(void *p_arg) {
    OS_ERR err;
    CPU_TS ticks;

    // CAN Commands
    CANDATA_t driveCmd = {
        .ID   = MOTOR_DRIVE,
        .idx  = 0,
        .data = {0.0f, 0.0f},
    };
    CANDATA_t powerCmd = {
        .ID   = MOTOR_POWER,
        .idx  = 0,
        .data = {0.0f, 0.0f},
    };

    while (1) {
        readInputs(); // read inputs from the system

        updateDisplayState();

        // Check that motor is ready to run (non-blocking)
        OSFlagPend(&BPS_Motor_Status_Flags, BPS_SAFE | BPS_CHECKED | MOTOR_SAFE_TO_RUN, 0,
                   OS_OPT_PEND_FLAG_SET_ALL | OS_OPT_PEND_NON_BLOCKING, &ticks, &err);
        // if you return OS_ERR_PEND_WOULD_BLOCK, one of the bits are not sent, and would've blocked
        if (err != OS_ERR_PEND_WOULD_BLOCK) {
            assertOSError(err);
        }

        // All bits are set
        if (err == OS_ERR_NONE) {
            // CAN message for setpoint of bus current percent
            memcpy(&powerCmd.data[4], &busCurrentSetPoint, sizeof(float));
            CANbus_Send(powerCmd, CAN_BLOCKING, MOTORCAN);

            // Update velocitySetpoint & currentSetpoint based on gear/state
            // NOTE: the brakePedalPercent checks when setting currentSetpoint are for hysteresis
            switch (gear) {
                case DASH_FWD:
                    velocitySetpoint = MAX_VELOCITY;
                    currentSetpoint  = isBrakeOn
                                           ? 0
                                           : mapToPercent(accelPedalPercent, ACCEL_PEDAL_THRESHOLD,
                                                          PEDAL_MAX, CURRENT_SP_MIN, CURRENT_SP_MAX);
                    break;
                case DASH_NEU:
                    velocitySetpoint = MAX_VELOCITY;
                    currentSetpoint  = 0.0f;
                    break;
                case DASH_REV:
                    velocitySetpoint = -MAX_VELOCITY;
                    currentSetpoint  = isBrakeOn
                                           ? 0
                                           : mapToPercent(accelPedalPercent, ACCEL_PEDAL_THRESHOLD,
                                                          PEDAL_MAX, CURRENT_SP_MIN, CURRENT_SP_MAX);
                    break;
                default:
                    assertSendTritiumError(C_ERR_STR_GEAR_FAULT);
                    break;
            }
        }

        // Motor is not safe to run so velocitySetpoint and currentSetpoint are set to 0
        else {
            velocitySetpoint = 0;
            currentSetpoint = 0.0f;
        }
        memcpy(&driveCmd.data[4], &currentSetpoint, sizeof(float));
        memcpy(&driveCmd.data[0], &velocitySetpoint, sizeof(float));
        CANbus_Send(driveCmd, CAN_BLOCKING, MOTORCAN);
        SendCarCAN_Put(driveCmd);

        // Delay of FSM_PERIOD ms
        OSTimeDlyHMSM(0, 0, 0, FSM_PERIOD, OS_OPT_TIME_HMSM_STRICT, &err);
        assertOSError(err);
    }
}

static void assertSendTritiumError(controls_error_e sterr) {
    switch (sterr) {
        case C_ERR_NONE:
            break;
        case C_ERR_STR_GENERIC:
        case C_ERR_STR_GEAR_FAULT:
            throwTaskError(sterr, false, NULL, OPT_LOCK_SCHED, OPT_NONRECOV);
            break;
        default:
            // Critical failure, we have a non sendtritium error in send tritium somehow
            throwTaskError(C_ERR_ILLEGAL_ERROR, false, NULL, OPT_LOCK_SCHED, OPT_NONRECOV);
            break;
    }
}