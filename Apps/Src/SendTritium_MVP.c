/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file SendTritium.c
 * @brief Function implementations for the SendTritium application.
 *
 * This contains functions relevant to updating the velocity and current setpoitns
 * of the Tritium motor controller. The implementation includes a normal current
 * controlled mode, a one-pedal driving mode (with regenerative braking), and cruise control.
 * The logic is determined through a finite state machine implementation.
 *
 * If the macro SENDTRITIUM_EXPOSE_VARS is defined prior to including SendTritium.h,
 * relevant setters will be exposed as externs for unit testing
 * and hardware inputs won't be read and motor commands won't be sent over MotorCAN.
 * If the macro SENDTRITIUM_PRINT_MES is also defined prior to including SendTritium.h,
 * debug info will be printed via UART.
 */

#include "Pedals.h"
#include "ReadCarCAN.h"
#include "Minions.h"
#include "ReadTritium.h"
#include "SendCarCAN.h"
#include "CANbus.h"
#include "UpdateDisplay.h"
#include "CANConfig.h"
#include "common.h"

#include "SendTritium_MVP.h"

// Macros
#define MAX_VELOCITY 20000.0f // rpm (unobtainable value)

#define MIN_CRUISE_VELOCITY mpsToRpm(20.0f)    // rpm
#define MAX_GEARSWITCH_VELOCITY mpsToRpm(8.0f) // rpm

#define BRAKE_PEDAL_THRESHOLD 50 // percent
#define ACCEL_PEDAL_THRESHOLD 15 // percent

#define ONEPEDAL_BRAKE_THRESHOLD 25   // percent
#define ONEPEDAL_NEUTRAL_THRESHOLD 35 // percent

#define PEDAL_MIN 0        // percent
#define PEDAL_MAX 100      // percent
#define CURRENT_SP_MIN 0   // percent
#define CURRENT_SP_MAX 100 // percent

#define GEAR_FAULT_THRESHOLD 3 // number of times gear fault can occur before it is considered a fault

// Inputs
static uint8_t brakePedalPercent = 0;
static uint8_t accelPedalPercent = 0;

static Gear_t gear = PARK_GEAR;

// Outputs
float currentSetpoint = 0;
float velocitySetpoint = 0;
float busCurrentSetPoint = 69.0f; // why is this float and not int?

// Counter for sending setpoints to motor
static uint8_t motorMsgCounter = 0;

// Debouncing counters
// static uint8_t onePedalCounter = 0;
// static uint8_t cruiseEnableCounter = 0;
// static uint8_t cruiseSetCounter = 0;

// FSM
static TritiumState_t prevState; // Previous state
static TritiumState_t state;     // Current state

// Getter functions for local variables in SendTritium.c
GETTER(uint8_t, brakePedalPercent)
GETTER(uint8_t, accelPedalPercent)
GETTER(Gear_t, gear)
GETTER(TritiumState_t, state)
GETTER(float, currentSetpoint)
GETTER(float, velocitySetpoint)

// Setter functions for local variables in SendTritium.c
#ifdef SENDTRITIUM_EXPOSE_VARS
SETTER(uint8_t, brakePedalPercent)
SETTER(uint8_t, accelPedalPercent)
SETTER(Gear_t, gear)
SETTER(TritiumState_t, state)
SETTER(float, currentSetpoint)
SETTER(float, velocitySetpoint)
#endif

// Handler & Decider Declarations
static void ForwardDriveHandler(void);
static void ForwardDriveDecider(void);
static void ParkDriveHandler(void);
static void ParkDriveDecider(void);
static void ReverseDriveHandler(void);
static void ReverseDriveDecider(void);

// FSM
static const TritiumState_t FSM[9] = {
    {FORWARD_DRIVE, &ForwardDriveHandler, &ForwardDriveDecider},
    {PARK_STATE, &ParkHandler, &ParkDecider},
    {REVERSE_DRIVE, &ReverseDriveHandler, &ReverseDriveDecider}};

// Helper Functions

/**
 * @brief Converts integer percentage to float percentage
 * @param percent integer percentage from 0-100
 * @returns float percentage from 0.0-1.0
 */
extern const float pedalToPercent[];
static float percentToFloat(uint8_t percent)
{
    if (percent > 100)
    {
        return 1.0f;
    }
    return pedalToPercent[percent];
}

#ifdef SENDTRITIUM_PRINT_MES
/**
 * @brief Dumps info to UART during testing
 */
static void getName(char *nameStr, uint8_t stateNameNum)
{
    switch (stateNameNum)
    {
    case FORWARD_DRIVE:
        strcpy(nameStr, "FORWARD_DRIVE");
        break;
    case PARK_STATE:
        strcpy(nameStr, "PARK_STATE");
        break;
    case REVERSE_DRIVE:
        strcpy(nameStr, "REVERSE_DRIVE");
        break;
    default:
        strcpy(nameStr, "UNKNOWN");
        break;
    }
    return;
}

static void dumpInfo()
{
    printf("-------------------\n\r");
    char stateName[20];
    getName(stateName, state.name);
    printf("State: %s\n\r", stateName);
    printf("brakePedalPercent: %d\n\r", brakePedalPercent);
    printf("accelPedalPercent: %d\n\r", accelPedalPercent);
    printf("gear: %d\n\r", (uint8_t)gear);
    print_float("currentSetpoint: ", currentSetpoint);
    print_float("velocitySetpoint: ", velocitySetpoint);
    printf("-------------------\n\r");
}
#endif

#ifndef SENDTRITIUM_EXPOSE_VARS
/**
 * @brief Reads inputs from the system
 */
static void readInputs()
{

    // Update pedals
    static uint32_t brakeSaturationCt = 0;

    uint32_t latest_pedal = Pedals_Read(BRAKE);

    if (brakeSaturationCt < 3)
    {
        if (latest_pedal == 100)
            brakeSaturationCt++;
        else if (latest_pedal == 0 && brakeSaturationCt != 0)
            brakeSaturationCt--;

        brakePedalPercent = 0;
    }
    else if (brakeSaturationCt >= 3)
    {
        brakePedalPercent = 100;

        if (latest_pedal == 0)
            brakeSaturationCt = 0;
    }

    accelPedalPercent = Pedals_Read(ACCELERATOR);

    // Update gears
    bool forwardSwitch = Minions_Read(FOR_SW);
    bool reverseSwitch = Minions_Read(REV_SW);
    bool forwardGear = (forwardSwitch && !reverseSwitch);
    bool reverseGear = (!forwardSwitch && reverseSwitch);
    bool parkGear = (!forwardSwitch && !reverseSwitch);

    uint8_t gearFault = (uint8_t)forwardGear + (uint8_t)reverseGear + (uint8_t)parkGear;
    static uint8_t gearFaultCnt = 0;

    if (gearFault != 1)
    {
        // Fault behavior
        if (gearFaultCnt > GEAR_FAULT_THRESHOLD)
            state = FSM[PARK_STATE];
        else
            gearFaultCnt++;
    }
    else
    {
        gearFaultCnt = 0;
    }

    if (parkGear)
        gear = PARK_GEAR;
    else if (forwardGear)
        gear = FORWARD_GEAR;
    else if (reverseGear)
        gear = REVERSE_GEAR;
    else
        gear = PARK_GEAR;

    if (gear == PARK_GEAR)
        UpdateDisplay_SetGear(DISP_PARK);
    else if (gear == FORWARD_GEAR)
        UpdateDisplay_SetGear(DISP_FORWARD);
    else if (gear == REVERSE_GEAR)
        UpdateDisplay_SetGear(DISP_REVERSE);
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
static uint8_t map(uint8_t input, uint8_t in_min, uint8_t in_max, uint8_t out_min, uint8_t out_max)
{
    if (in_min >= in_max)
        in_max = in_min; // The minimum of the input range should never be greater than the maximum of the input range

    if (input <= in_min)
    {
        // Lower bound the input to the minimum possible output
        return out_min;
    }
    else if (input >= in_max)
    {
        // Upper bound the input to the maximum output
        return out_max;
    }
    else
    {
        // Linear mapping between ranges
        uint8_t offset_in = input - in_min;    // If input went from A -> B, it now goes from 0 -> B-A
        uint8_t in_range = in_max - in_min;    // Input range
        uint8_t out_range = out_max - out_min; // Output range
        uint8_t offset_out = out_min;
        return (offset_in * out_range) / in_range + offset_out; // slope = out_range/in_range. y=mx+b so output=slope*offset_in+offset_out
    }
}

// State Handlers & Deciders

/**
 * @brief Forward Drive State Handler. Accelerator is mapped directly
 * to current setpoint at positive velocity.
 */
static void ForwardDriveHandler()
{
    velocitySetpoint = MAX_VELOCITY;
    // printf("accelPedalPercent: %d\n\r", accelPedalPercent);
    currentSetpoint = percentToFloat(map(accelPedalPercent, ACCEL_PEDAL_THRESHOLD, PEDAL_MAX, CURRENT_SP_MIN, CURRENT_SP_MAX));
    // print_float("float(map(accelPedalPercent)): %d\n\r", currentSetpoint);
}

/**
 * @brief Forward Drive State Decider. Determines transitions out of
 * forward drive state (brake, record velocity, one pedal, neutral drive).
 */
static void ForwardDriveDecider()
{
    if (brakePedalPercent >= BRAKE_PEDAL_THRESHOLD)
    {
        state = FSM[PARK_STATE];
        Minions_Write(BRAKELIGHT, true);
    }
    else if (gear == REVERSE_GEAR)
    {
        state = FSM[PARK_STATE]; // transitions to PARK_STATE to then transition to REVERSE_DRIVE
        Minions_Write(BRAKELIGHT, false);
    }
    // Otherwise, stays in FORWARD_DRIVE
}

/**
 * @brief Neutral Drive State Handler. No current is sent to the motor.
 */
static void ParkHandler()
{
    velocitySetpoint = MAX_VELOCITY;
    currentSetpoint = 0.0f;
    if(brakePedalPercent >= BRAKE_PEDAL_THRESHOLD)
        Minions_Write(BRAKELIGHT, true);    
}

/**
 * @brief Neutral Drive State Decider. Determines transitions out of
 * neutral drive state (brake, forward drive, reverse drive).
 */
static void ParkDecider()
{
    if (brakePedalPercent >= BRAKE_PEDAL_THRESHOLD)
    {
        state = FSM[PARK_STATE];
        Minions_Write(BRAKELIGHT, true);
    }
    else if (gear == FORWARD_GEAR)
    {
        state = FSM[FORWARD_DRIVE];
        Minions_Write(BRAKELIGHT, false);
    }
    else if (gear == REVERSE_GEAR)
    {
        state = FSM[REVERSE_DRIVE];
        Minions_Write(BRAKELIGHT, false);
    }
}

/**
 * @brief Reverse Drive State Handler. Accelerator is mapped directly to
 * current setpoint (at negative velocity).
 */
static void ReverseDriveHandler()
{
    velocitySetpoint = -MAX_VELOCITY;
    currentSetpoint = percentToFloat(map(accelPedalPercent, ACCEL_PEDAL_THRESHOLD, PEDAL_MAX, CURRENT_SP_MIN, CURRENT_SP_MAX));
    if(brakePedalPercent >= BRAKE_PEDAL_THRESHOLD)
        Minions_Write(BRAKELIGHT, true);    
}

/**
 * @brief Reverse Drive State Decider. Determines transitions out of
 * reverse drive state (brake, neutral drive).
 */
static void ReverseDriveDecider()
{
    if (brakePedalPercent >= BRAKE_PEDAL_THRESHOLD)
    {
        state = FSM[PARK_STATE];
        Minions_Write(BRAKELIGHT, true);
    }
    else if (gear == FORWARD_GEAR)
    {
        state = FSM[PARK_STATE]; // transitions to PARK_STATE to then transition to FORWARD_DRIVE
        Minions_Write(BRAKELIGHT, false);
    }
    // Otherwise, stays in REVERSE_DRIVE
}

// Task (main loop)

/**
 * @brief Follows the FSM to update the velocity of the car
 */
void Task_SendTritium(void *p_arg)
{
    OS_ERR err;

    // Initialize current state to PARK_STATE
    state = FSM[PARK_STATE];
    prevState = FSM[PARK_STATE];

#ifndef SENDTRITIUM_EXPOSE_VARS
    CANDATA_t driveCmd = {
        .ID = MOTOR_DRIVE,
        .idx = 0,
        .data = {0.0f, 0.0f},
    };
#endif
    CANDATA_t powerCmd = {
        .ID = MOTOR_POWER,
        .idx = 0,
        .data = {0.0f, 0.0f},
    };

    while (1)
    {
        memcpy(&powerCmd.data[4], &busCurrentSetPoint, sizeof(float));
        CANbus_Send(powerCmd, CAN_BLOCKING, MOTORCAN); //<-------
        state.stateHandler();                          // do what the current state does
#ifndef SENDTRITIUM_EXPOSE_VARS
        readInputs(); // read inputs from the system
        UpdateDisplay_SetAccel(accelPedalPercent);
#endif
        prevState = state;
        state.stateDecider(); // decide what the next state is

        // Disable velocity controlled mode by always overwriting velocity to the maximum
        // in the appropriate direction.
        velocitySetpoint = (velocitySetpoint > 0) ? MAX_VELOCITY : -MAX_VELOCITY;

// Drive
#ifdef SENDTRITIUM_PRINT_MES
        dumpInfo();
#endif
#ifndef SENDTRITIUM_EXPOSE_VARS
        if (MOTOR_MSG_COUNTER_THRESHOLD == motorMsgCounter)
        {
            memcpy(&driveCmd.data[4], &currentSetpoint, sizeof(float));
            memcpy(&driveCmd.data[0], &velocitySetpoint, sizeof(float));
            CANbus_Send(driveCmd, CAN_NON_BLOCKING, MOTORCAN);
            motorMsgCounter = 0;
        }
        else
        {
            motorMsgCounter++;
        }
#endif

        // Delay of FSM_PERIOD ms
        OSTimeDlyHMSM(0, 0, 0, FSM_PERIOD, OS_OPT_TIME_HMSM_STRICT, &err);
        if (err != OS_ERR_NONE)
        {
            assertOSError(err);
        }
    }
}
