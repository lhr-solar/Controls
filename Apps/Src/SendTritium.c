/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file SendTritium.c
 * @brief Function implementations for the SendTritium application.
 *
 * This contains functions relevant to updating the velocity and current
 * setpoitns of the Tritium motor controller. The implementation includes a
 * normal current controlled mode, a one-pedal driving mode (with regenerative
 * braking), and cruise control. The logic is determined through a finite state
 * machine implementation.
 *
 * If the macro SENDTRITIUM_EXPOSE_VARS is defined prior to including
 * SendTritium.h, relevant setters will be exposed as externs for unit testing
 * and hardware inputs won't be read and motor commands won't be sent over
 * MotorCAN. If the macro SENDTRITIUM_PRINT_MES is also defined prior to
 * including SendTritium.h, debug info will be printed via UART.
 */

#include "SendTritium.h"

#include "CanBus.h"
#include "CanConfig.h"
#include "Minions.h"
#include "Pedals.h"
#include "ReadCarCan.h"
#include "ReadTritium.h"
#include "SendCarCan.h"
#include "UpdateDisplay.h"
#include "common.h"

// Macros
#define MAX_VELOCITY 20000.0f  // rpm (unobtainable value)

#define MAX_GEARSWITCH_VELOCITY MpsToRpm(8.0f)  // rpm

#define BRAKE_PEDAL_THRESHOLD 50  // percent
#define ACCEL_PEDAL_THRESHOLD 10  // percent

#define PEDAL_MIN 0         // percent
#define PEDAL_MAX 100       // percent
#define CURRENT_SP_MIN 0    // percent
#define CURRENT_SP_MAX 100  // percent

#define GEAR_FAULT_THRESHOLD \
    3  // number of times gear fault can occur before it is considered a fault

// Macros for software
#define NUM_STATES 3

// Inputs
static uint8_t brake_pedal_percent = 0;
static uint8_t accel_pedal_percent = 0;

static Gear gear = kParkGear;

// Outputs
float current_setpoint = 0;
float velocity_setpoint = 0;

// Current observed velocity
static float velocity_observed = 0;

#ifndef SENDTRITIUM_EXPOSE_VARS
// Counter for sending setpoints to motor
static uint8_t motor_msg_counter = 0;

#endif

// FSM
static TritiumState prev_state;  // Previous state
static TritiumState state;       // Current state

uint8_t GetBrakePedalPercent(void) { return brake_pedal_percent; }
uint8_t GetAccelPedalPercent(void) { return accel_pedal_percent; }
Gear GetGear(void) { return gear; }
TritiumState GetState(void) { return state; }
float GetVelocityObserved(void) { return velocity_observed; }
float GetCurrentSetpoint(void) { return current_setpoint; }
float GetVelocitySetpoint(void) { return velocity_setpoint; }

#ifdef SENDTRITIUM_EXPOSE_VARS
void SetBrakePedalPercent(uint8_t value) { brake_pedal_percent = value; }
void SetAccelPedalPercent(uint8_t value) { accel_pedal_percent = value; }
void SetGear(Gear value) { gear = value; }
void SetState(TritiumState value) { state = value; }
void SetVelocityObserved(float value) { velocity_observed = value; }
void SetCurrentSetpoint(float value) { current_setpoint = value; }
void SetVelocitySetpoint(float value) { velocity_setpoint = value; }
#endif

// Handler & Decider Declarations
static void forwardDriveHandler(void);
static void forwardDriveDecider(void);
static void parkHandler(void);
static void parkDecider(void);
static void reverseDriveHandler(void);
static void reverseDriveDecider(void);

// FSM
static const TritiumState kFsm[NUM_STATES] = {
    {kForwardDrive, &forwardDriveHandler, &forwardDriveDecider},
    {kPark, &parkHandler, &parkDecider},
    {kReverseDrive, &reverseDriveHandler, &reverseDriveDecider},
};

// Helper Functions

/**
 * @brief Converts integer percentage to float percentage
 * @param percent integer percentage from 0-100
 * @returns float percentage from 0.0-1.0
 */
extern const float kPedalToPercent[];
static float percentToFloat(uint8_t percent) {
    if (percent > 100) {
        return 1.0F;
    }
    return kPedalToPercent[percent];
}

// Print-debugging functionality
#ifdef SENDTRITIUM_PRINT_MES

#define STATE_NAME_STR_SIZE 20

/**
 * @brief Dumps info to UART during testing
 */
static void getName(char* name_str, uint8_t state_name_num) {
    switch (state_name_num) {
        case kForwardDrive:
            strcpy(name_str, "FORWARD_DRIVE");
            break;
        case kPark:
            strcpy(name_str, "PARK");
            break;
        case kReverseDrive:
            strcpy(name_str, "REVERSE_DRIVE");
            break;
        default:
            strcpy(name_str, "UNKNOWN");
            break;
    }
}

static void dumpInfo() {
    printf("-------------------\n\r");
    char state_name[STATE_NAME_STR_SIZE];
    getName(state_name, state.name);
    printf("State: %s\n\r", state_name);
    printf("brakePedalPercent: %d\n\r", brake_pedal_percent);
    printf("accelPedalPercent: %d\n\r", accel_pedal_percent);
    printf("gear: %d\n\r", (uint8_t)gear);
    PrintFloat("currentSetpoint: ", current_setpoint);
    PrintFloat("velocitySetpoint: ", velocity_setpoint);
    PrintFloat("velocityObserved: ", velocity_observed);
    printf("-------------------\n\r");
}
#endif

#ifndef SENDTRITIUM_EXPOSE_VARS
/**
 * @brief Reads inputs from the system
 */
static void readInputs() {
    // Update pedals
    brake_pedal_percent = PedalsRead(kBrake);
    accel_pedal_percent = PedalsRead(kAccelerator);

    // Update gears
    bool forward_switch = MinionsRead(kForSw);
    bool reverse_switch = MinionsRead(kRevSw);
    bool forward_gear = (forward_switch && !reverse_switch);
    bool reverse_gear = (!forward_switch && reverse_switch);
    bool park_gear = (!forward_switch && !reverse_switch);

    uint8_t gear_fault =
        (uint8_t)forward_gear + (uint8_t)reverse_gear + (uint8_t)park_gear;
    static uint8_t gear_fault_cnt = 0;

    if (gear_fault != 1) {
        // Fault behavior
        if (gear_fault_cnt > GEAR_FAULT_THRESHOLD) {
            state = kFsm[kPark];
        } else {
            gear_fault_cnt++;
        }
    } else {
        gear_fault_cnt = 0;
    }

    if (forward_gear) {
        gear = kForwardGear;
    } else if (reverse_gear) {
        gear = kReverseGear;
    } else {
        gear = kParkGear;
    }

    // Get observed velocity
    velocity_observed = MotorRpmGet();
}
#endif

/**
 * @brief Linearly map range of integers to another range of integers.
 * in_min to in_max is mapped to out_min to out_max.
 * @param input input integer value
 * @param in_min minimum value of input range
 * @param in_max maximum value of input range
 * @param out_min minimum value of output range
 * @param out_max maximum value of output range
 * @returns integer value from out_min to out_max
 */
static uint8_t map(uint8_t input, uint8_t in_min, uint8_t in_max,
                   uint8_t out_min, uint8_t out_max) {
    if (in_min >= in_max) {
        in_max = in_min;  // The minimum of the input range should never be
                          // greater than the maximum of the input range
    }
    if (input <= in_min) {
        // Lower bound the input to the minimum possible output
        return out_min;
    }
    if (input >= in_max) {
        // Upper bound the input to the maximum output
        return out_max;
    }  // Linear mapping between ranges
    uint8_t offset_in =
        input - in_min;  // If input went from A -> B, it now goes from 0 -> B-A
    uint8_t in_range = in_max - in_min;     // Input range
    uint8_t out_range = out_max - out_min;  // Output range
    uint8_t offset_out = out_min;
    return (offset_in * out_range) / in_range +
           offset_out;  // slope = out_range/in_range. y=mx+b so
                        // output=slope*offset_in+offset_out
}

/**
 * @brief Put the kControlMode message onto the CarCAN bus, detailing
 * the current mode of control.
 */
static void putControlModeCAN() {
    CanData message;
    memset(&message, 0, sizeof(message));
    message.id = kControlMode;
    message.data[0] = state.name;

    SendCarCanPut(message);
}

// State Handlers & Deciders

/**
 * @brief Forward Drive State Handler. Accelerator is mapped directly
 * to current setpoint at positive velocity.
 */
static void forwardDriveHandler() {
    if (prev_state.name != state.name) {
        UpdateDisplaySetGear(DISP_FORWARD);
    }
    velocity_setpoint = MAX_VELOCITY;
    current_setpoint =
        percentToFloat(map(accel_pedal_percent, ACCEL_PEDAL_THRESHOLD,
                           PEDAL_MAX, CURRENT_SP_MIN, CURRENT_SP_MAX));
    // Turn on brake lights if mechanical brake is pressed
    if (brake_pedal_percent >= BRAKE_PEDAL_THRESHOLD) {
        MinionsWrite(kBrakeLight, true);
    }
}

/**
 * @brief Forward Drive State Decider. Determines transitions out of
 * forward drive state.
 */
static void forwardDriveDecider() {
    if (brake_pedal_percent >= BRAKE_PEDAL_THRESHOLD) {
        if (gear == kParkGear || gear == kReverseGear) {
            state = kFsm[kPark];
        }
    } else {
        state = kFsm[kForwardDrive];
    }
}

/**
 * @brief Neutral Drive State Handler. No current is sent to the motor.
 */
static void parkHandler() {
    if (prev_state.name != state.name) {
        UpdateDisplaySetGear(DISP_PARK);
    }
    velocity_setpoint = MAX_VELOCITY;
    current_setpoint = 0.0F;
    // Turn on brake lights if mechanical brake is pressed
    if (brake_pedal_percent >= BRAKE_PEDAL_THRESHOLD) {
        MinionsWrite(kBrakeLight, true);
    }
}

/**
 * @brief Neutral Drive State Decider. Determines transitions out of
 * neutral drive state.
 */
static void parkDecider() {
    if (gear == kForwardGear) {
        state = kFsm[kForwardDrive];
    } else if (gear == kReverseGear) {
        state = kFsm[kReverseDrive];
    } else {
        state = kFsm[kPark];
    }
}

/**
 * @brief Reverse Drive State Handler. Accelerator is mapped directly to
 * current setpoint (at negative velocity).
 */
static void reverseDriveHandler() {
    if (prev_state.name != state.name) {
        UpdateDisplaySetGear(DISP_REVERSE);
    }
    velocity_setpoint = -MAX_VELOCITY;
    current_setpoint =
        percentToFloat(map(accel_pedal_percent, ACCEL_PEDAL_THRESHOLD,
                           PEDAL_MAX, CURRENT_SP_MIN, CURRENT_SP_MAX));
    // Turn on brake lights if mechanical brake is pressed
    if (brake_pedal_percent >= BRAKE_PEDAL_THRESHOLD) {
        MinionsWrite(kBrakeLight, true);
    }
}

/**
 * @brief Reverse Drive State Decider. Determines transitions out of
 * reverse drive state.
 */
static void reverseDriveDecider() {
    if (brake_pedal_percent >= BRAKE_PEDAL_THRESHOLD) {
        if (gear == kParkGear || gear == kForwardGear) {
            state = kFsm[kPark];
        }
    } else {
        state = kFsm[kReverseDrive];
    }
}

// Task (main loop)

/**
 * @brief Follows the FSM to update the velocity of the car
 */
void TaskSendTritium(void* p_arg) {
    OS_ERR err = 0;

    // Initialize current state to PARK
    state = kFsm[kPark];
    prev_state = kFsm[kPark];

#ifndef SENDTRITIUM_EXPOSE_VARS
    CanData drive_cmd = {
        .id = kMotorDrive,
        .idx = 0,
        .data = {0},
    };
#endif

    while (1) {
        prev_state = state;

        state.stateHandler();  // do what the current state does
#ifndef SENDTRITIUM_EXPOSE_VARS
        readInputs();  // read inputs from the system
        UpdateDisplaySetAccel(accel_pedal_percent);
#endif
        state.stateDecider();  // decide what the next state is

        // Disable velocity controlled mode by always overwriting velocity to
        // the maximum in the appropriate direction.
        velocity_setpoint =
            (velocity_setpoint > 0) ? MAX_VELOCITY : -MAX_VELOCITY;

// Drive
#ifdef SENDTRITIUM_PRINT_MES
        dumpInfo();
#endif
#ifndef SENDTRITIUM_EXPOSE_VARS
        if (MOTOR_MSG_COUNTER_THRESHOLD == motor_msg_counter) {
            memcpy(&drive_cmd.data[4], &current_setpoint, sizeof(float));
            memcpy(&drive_cmd.data[0], &velocity_setpoint, sizeof(float));
            CanBusSend(drive_cmd, CAN_NON_BLOCKING, MOTORCAN);
            motor_msg_counter = 0;
        } else {
            motor_msg_counter++;
        }
#endif

        putControlModeCAN();

        // Delay of MOTOR_MSG_PERIOD ms
        OSTimeDlyHMSM(0, 0, 0, MOTOR_MSG_PERIOD, OS_OPT_TIME_HMSM_STRICT, &err);
        if (err != OS_ERR_NONE) {
            ASSERT_OS_ERROR(err);
        }
    }
}
