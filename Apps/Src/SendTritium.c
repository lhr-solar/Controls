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
#include "SendCarCAN.h"
#include "SendTritium.h"
#include "Tasks.h"
#include "DebugIO.h"
#include "UpdateDisplay.h"
#include "Lights.h"

// #define USING_PROFINITY

// Inputs
static uint8_t brakePedalPercent = 0;
static uint8_t accelPedalPercent = 0;
static gear_t gear = DASH_NEU;
static bool isBrakeOn = false; // Used for updating display & brakelight

// Outputs
static float currentSetpoint = 0.0f;
static float velocitySetpoint = 0.0f;
static float busCurrentSetPoint = 1.0f;

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
}

/**
 * @brief Reads inputs from the system
 */
static void readInputs() {
    brakePedalPercent = Pedals_Read(BRAKE);
    accelPedalPercent = Pedals_Read(ACCELERATOR);
    CANDATA_t rawPedalmv = {
        .ID = PEDALS_RAW_VOLTAGE, 
        .idx = 0, 
        .data = {0}
    };

    ((int16_t*)rawPedalmv.data)[0] = Pedals_rawVoltage(BRAKE);
    ((int16_t*)rawPedalmv.data)[1] = Pedals_rawVoltage(ACCELERATOR);
    SendCarCAN_Put(rawPedalmv);

    // // Brake hysteresis
    if (brakePedalPercent >= BRAKE_PRESSED_THRESHOLD)
        isBrakeOn = true;
    else if (brakePedalPercent <= BRAKE_UNPRESSED_THRESHOLD)
        isBrakeOn = false;
    Lights_Write(BRAKE_LIGHT, isBrakeOn);
    gear = getGear(GEAR_USE_OS_DELAY);

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
        uint8_t offset_in = input - in_min; // If input went from A -> B, it now goes from 0 -> B-A
        uint8_t in_range = in_max - in_min; // Input range
        uint8_t out_range = out_max - out_min; // Output range
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
    // CPU_TS ticks;

    // CAN Commands
    CANDATA_t driveCmd = {
        .ID = MOTOR_DRIVE,
        .idx = 0,
        .data = {0.0f, 0.0f}
    };
    CANDATA_t powerCmd = {
        .ID = MOTOR_POWER,
        .idx = 0,
        .data = {0.0f, 0.0f}
    };

    CANDATA_t motorSafeCmd = {
        .ID = MOTOR_CONTROLLER_SAFE, 
        .idx = 0, 
        .data = {0}
    };

    // Accelerator is pulled to GND and then inverted in code so when nothing is plugged in accel defaults to 100%
    // In order to spin the motor the accelerator needs to return to a safe value
    static bool accelerator_reset = false;
    uint8_t resetAccelPercent = 0;

    while (1) {
        #ifdef TASK_PROFILER
        DebugIO_Toggle(SEND_TRITIUM_PIN);
        #endif
        readInputs(); // read inputs from the system

        updateDisplayState();

        // Check that motor is ready to run
        err = MotorStatus_Wait(BPS_SAFE | BPS_CHECKED | MOTOR_SAFE_TO_RUN, !OS_FLAG_BLOCKING);

        // if you return OS_ERR_PEND_WOULD_BLOCK, one of the bits are not sent, and would've blocked
        // If the error is ERR_NONE, assertOSError returns without asserting an error
        if (err != OS_ERR_PEND_WOULD_BLOCK) {
            assertOSError(err);
        }

        memset(&motorSafeCmd.data, 0, sizeof(motorSafeCmd.data));
        // All bits are set
        if (err == OS_ERR_NONE) {
            // CAN message for setpoint of bus current percent
            memcpy(&powerCmd.data[4], &busCurrentSetPoint, sizeof(float));

            // If we're using the profinity software don't set the power here
#ifndef USING_PROFINITY
                CANbus_Send(powerCmd, CAN_BLOCKING, MOTORCAN);
#endif
            // The accelerator at some point has been lowered to a safe value
            if(accelerator_reset){
                resetAccelPercent = accelPedalPercent;
            }
            // The accelerator is now at a safe value
            else if(!accelerator_reset && accelPedalPercent <= ACCCEL_PEDAL_RESET_THRESHOLD){
                accelerator_reset = true;
                resetAccelPercent = accelPedalPercent;
            }
            // The accelerator has ever been set to a safe value
            else{
                resetAccelPercent = 0;
            }


            switch (gear) {
                case DASH_FWD:
                    velocitySetpoint = MAX_VELOCITY;
\                    currentSetpoint = isBrakeOn ? 0 : (mapToPercent(resetAccelPercent, ACCEL_PEDAL_THRESHOLD, PEDAL_MAX, CURRENT_SP_MIN, CURRENT_SP_MAX));                 
                    break;

                case DASH_NEU:
                    velocitySetpoint = 0.0f;
                    currentSetpoint = 0.0f;
                    break;

                case DASH_REV:
                    velocitySetpoint = -MAX_VELOCITY;
                    currentSetpoint = isBrakeOn ? 0 : (mapToPercent(resetAccelPercent, ACCEL_PEDAL_THRESHOLD, PEDAL_MAX, CURRENT_SP_MIN, CURRENT_SP_MAX));   
                    break;

                default:
                    assertSendTritiumError(C_ERR_STR_GEAR_FAULT);
                    break;
            }
            motorSafeCmd.data[0] |= 0x01; // Set motor safe to run to true
        }

        // Motor is not safe to run so velocitySetpoint and currentSetpoint are set to 0
        else {
            velocitySetpoint = 0.0f;
            currentSetpoint = 0.0f;
            motorSafeCmd.data[0] = 0; // Set motor safe to run to false
        }
        memcpy(&driveCmd.data[4], &currentSetpoint, sizeof(float));
        memcpy(&driveCmd.data[0], &velocitySetpoint, sizeof(float));

        // Set motor fault as 0
        // The motor controller will error if 2 different sources are sending drive commands, so if
        // profinity is plugged in, don't send drive command
        #ifndef USING_PROFINITY
                // Drive command must be sent every 250ms or the motor will return to neutral
                CANbus_Send(driveCmd, CAN_BLOCKING, MOTORCAN);
        #endif
        SendCarCAN_Put(motorSafeCmd); // Send the motor safe command
        SendCarCAN_Put(driveCmd); // Send the drive command to the car CAN bus for telemetry

        // Delay of FSM_PERIOD ms
        OSTimeDlyHMSM(0, 0, 0, FSM_PERIOD, OS_OPT_TIME_HMSM_STRICT, &err);
        assertOSError(err);
        #ifdef TASK_PROFILER
        DebugIO_Toggle(SEND_TRITIUM_PIN);
        #endif
    }
}

static void assertSendTritiumError(controls_error_e sterr) {
    switch (sterr) {
        case C_ERR_NONE:
            break;
        case C_ERR_STR_GENERIC:
        case C_ERR_STR_GEAR_FAULT:
            throwTaskError(sterr, false, NULL, OPT_LOCK_SCHED, OPT_NONRECOV, CAN_NONE_BPS);
            break;
        default:
            // Critical failure, we have a non sendtritium error in send tritium somehow
            throwTaskError(C_ERR_ILLEGAL_ERROR, false, NULL, OPT_LOCK_SCHED, OPT_NONRECOV, CAN_NONE_BPS);
            break;
    }
}