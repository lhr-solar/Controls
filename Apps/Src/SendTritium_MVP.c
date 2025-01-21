/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file SendTritium_MVP.c
 * @brief Function implementations for the SendTritium application for the MVP control
 * mode (normal current control).
 *
 * This contains functions relevant to updating the velocity and current setpoints
 * of the Tritium motor controller. The implementation includes only the normal current
 * control mode, not one pedal driving, regenerative braking, and cruise control capabilities/
 * modes. The logic is determined through a finite state machine implementation.
 *
 * If the macro SENDTRITIUM_MVP_EXPOSE_VARS is defined prior to including SendTritium_MVP.h,
 * relevant setters will be exposed as externs for unit testing and hardware inputs 
 * won't be read and motor commands won't be sent over MotorCAN.
 * If the macro SENDTRITIUM_MVP_PRINT_MES is also defined prior to including SendTritium_MVP.h,
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
#include "Tasks.h"

#include "SendTritium_MVP.h"

// Inputs
static uint8_t brakePedalPercent = 0;
static uint8_t accelPedalPercent = 0;

static Gear_t gear = PARK_GEAR;

// Outputs
static float currentSetpoint = 0.0f;
static float velocitySetpoint = 0.0f;
static float busCurrentSetPoint = 69.0f; // why is this float and not int?

// Current observed velocity
static float velocityObserved = 0.0f;

// Counter for sending setpoints to motor
static uint8_t motorMsgCounter = 0;

// FSM
static TritiumState_t state;     // Current state

// Getter functions for local variables in SendTritium_MVP.c
GETTER(uint8_t, brakePedalPercent)
GETTER(uint8_t, accelPedalPercent)
GETTER(Gear_t, gear)
TritiumStateName_t get_state(void) {return state.name;}
GETTER(float, velocityObserved)
GETTER(float, currentSetpoint)
GETTER(float, velocitySetpoint)

// Setter functions for local variables in SendTritium_MVP.c
#ifdef SENDTRITIUM_MVP_EXPOSE_VARS
SETTER(uint8_t, brakePedalPercent)
SETTER(uint8_t, accelPedalPercent)
SETTER(Gear_t, gear)
void set_state(TritiumStateName_t stateName) {state = FSM[stateName];}
SETTER(float, velocityObserved)
SETTER(float, currentSetpoint)
SETTER(float, velocitySetpoint)
#endif

// Handler & Decider Declarations
static void ForwardDriveHandler(void);
static void ForwardDriveDecider(void);
static void ParkHandler(void);
static void ParkDecider(void);
static void ReverseDriveHandler(void);
static void ReverseDriveDecider(void);

// Caller functions for state handlers & deciders in SendTritium_MVP.c
#ifdef SENDTRITIUM_MVP_EXPOSE_VARS
void callForwardDriveHandler(void)  {ForwardDriveHandler();}
void callForwardDriveDecider(void)  {ForwardDriveDecider();}
void callParkHandler(void)          {ParkHandler();}
void callParkDecider(void)          {ParkDecider();}
void callReverseDriveHandler(void); {ReverseDriveHandler();}
void callReverseDriveDecider(void); {ReverseDriveDecider();}
#endif

// Function prototypes
static void assertSendTritiumMVPError(SendTritium_MVP_error_code_t stmvperr);

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

#ifdef SENDTRITIUM_MVP_PRINT_MES
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
    print_float("velocityObserved: ", velocityObserved);
    printf("-------------------\n\r");
}
#endif

#ifndef SENDTRITIUM_MVP_EXPOSE_VARS
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

    // Protect from both brake and accel being pressed
    if (brakePedalPercent == 100 && accelPedalPercent == 100) accelPedalPercent = 0;
    
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
            assertSendTritiumMVPError(SENDTRITIUM_MVP_ERR_GEAR_FAULT);
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

    // Display is state-based, not gear based, to account for gearswitch constraints in state changes
    if (state.name == PARK_STATE)
        UpdateDisplay_SetGear(DISP_PARK);
    else if (state.name == FORWARD_DRIVE)
        UpdateDisplay_SetGear(DISP_FORWARD);
    else if (state.name == REVERSE_DRIVE)
        UpdateDisplay_SetGear(DISP_REVERSE);

    // Get observed velocity
    velocityObserved = Motor_RPM_Get();
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

void assertSendTritiumMVPError(SendTritium_MVP_error_code_t stmvp_err) {
    
    Error_SendTritium_MVP = (error_code_t) stmvp_err;
    
    switch (stmvp_err) 
    {
        case SENDTRITIUM_MVP_ERR_NONE:
            break;

        case SENDTRITIUM_MVP_ERR_GEAR_FAULT:
            throwTaskError(Error_SendTritium_MVP, , OPT_LOCK_SCHED, OPT_NONRECOV);
            break;
        
        default:
            break;
    }
}

// State Handlers & Deciders

/**
 * @brief Forward Drive State Handler. Accelerator is mapped directly
 * to current setpoint at positive velocity.
 */
static void ForwardDriveHandler()
{
    // If braking, set current to 0. Otherwise, set current based on accel
    if(brakePedalPercent >= BRAKE_PEDAL_THRESHOLD) {
        velocitySetpoint = MAX_VELOCITY;
        currentSetpoint = 0.0f;
    } else {
        velocitySetpoint = MAX_VELOCITY;
        currentSetpoint = percentToFloat(map(accelPedalPercent, ACCEL_PEDAL_THRESHOLD, PEDAL_MAX, CURRENT_SP_MIN, CURRENT_SP_MAX));
    }

    // Turn brakelight on/off
    if(brakePedalPercent >= BRAKE_PEDAL_THRESHOLD) {
        Minions_Write(BRAKELIGHT, true);
        UpdateDisplay_SetBrake(true);
    } else {
        Minions_Write(BRAKELIGHT, false);
        UpdateDisplay_SetBrake(false);
    }
}

/**
 * @brief Forward Drive State Decider. Determines transitions out of
 * forward drive state (park).
 */
static void ForwardDriveDecider()
{
    // Go to PARK_STATE if you're in another gear
    if (gear == PARK_GEAR || gear == REVERSE_GEAR)
    {
        state = FSM[PARK_STATE];
    }
    // Otherwise, stays in FORWARD_DRIVE
}

/**
 * @brief Park State Handler. No current is sent to the motor.
 */
static void ParkHandler()
{
    velocitySetpoint = MAX_VELOCITY;
    currentSetpoint = 0.0f;

    // Turn brakelight on/off
    if(brakePedalPercent >= BRAKE_PEDAL_THRESHOLD) {
        Minions_Write(BRAKELIGHT, true);
        UpdateDisplay_SetBrake(true);
    } else {
        Minions_Write(BRAKELIGHT, false);
        UpdateDisplay_SetBrake(false);
    }
}

/**
 * @brief Park State Decider. Determines transitions out of
 * neutral drive state (forward drive, reverse drive).
 */
static void ParkDecider()
{
    // Switch to FORWARD_DRIVE/REVERSE_DRIVE if in FORWARD_GEAR/REVERSE_GEAR & speed is less than MAX_GEARSWITCH_VELOCITY
    if (gear == FORWARD_GEAR && (velocityObserved < 0 ? (velocityObserved >= -MAX_GEARSWITCH_VELOCITY) : (velocityObserved <= MAX_GEARSWITCH_VELOCITY)))
    {
        state = FSM[FORWARD_DRIVE];
    }
    else if (gear == REVERSE_GEAR && (velocityObserved < 0 ? (velocityObserved >= -MAX_GEARSWITCH_VELOCITY) : (velocityObserved <= MAX_GEARSWITCH_VELOCITY)))
    {
        state = FSM[REVERSE_DRIVE];
    }
    // Otherwise, remain in PARK_STATE
}

/**
 * @brief Reverse Drive State Handler. Accelerator is mapped directly to
 * current setpoint (at negative velocity).
 */
static void ReverseDriveHandler()
{
    // If braking, set current to 0. Otherwise, set current based on accel
    if(brakePedalPercent >= BRAKE_PEDAL_THRESHOLD) {
        velocitySetpoint = MAX_VELOCITY;
        currentSetpoint = 0.0f;
    } else {
        velocitySetpoint = -MAX_VELOCITY;
        currentSetpoint = percentToFloat(map(accelPedalPercent, ACCEL_PEDAL_THRESHOLD, PEDAL_MAX, CURRENT_SP_MIN, CURRENT_SP_MAX));
    }

    // Turn brakelight on/off
    if(brakePedalPercent >= BRAKE_PEDAL_THRESHOLD) {
        Minions_Write(BRAKELIGHT, true);
        UpdateDisplay_SetBrake(true);
    } else {
        Minions_Write(BRAKELIGHT, false);
        UpdateDisplay_SetBrake(false);
    }  
}

/**
 * @brief Reverse Drive State Decider. Determines transitions out of
 * reverse drive state (park).
 */
static void ReverseDriveDecider()
{
    // Go to PARK_STATE if you're in another gear or if you're braking
    if (gear == PARK_GEAR || gear == FORWARD_GEAR)
    {
        state = FSM[PARK_STATE];
    }
    // Otherwise, stays in REVERSE_DRIVE
}

// Task (main loop)

/**
 * @brief Follows the FSM to update the velocity of the car
 */
void Task_SendTritium_MVP(void *p_arg)
{
    OS_ERR err;

    // Initialize current state to PARK_STATE
    state = FSM[PARK_STATE];
    UpdateDisplay_SetGear(PARK_GEAR);

    // Initialize Regen & Cruise disabled
    // NOTE: Regen & Display stay disabled on Daybreak MVP
    UpdateDisplay_SetRegenState(DISP_DISABLED);
    UpdateDisplay_SetCruiseState(DISP_DISABLED);

#ifndef SENDTRITIUM_MVP_EXPOSE_VARS
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
#ifndef SENDTRITIUM_MVP_EXPOSE_VARS
        readInputs(); // read inputs from the system
        UpdateDisplay_SetAccel(accelPedalPercent);
#endif
        state.stateDecider(); // decide what the next state is

        // NOTE: This ONLY works for MVP (since Cruise has velocity control)
        // Disable velocity controlled mode (though this should never be needed, but just in 
        //case) by always overwriting velocity to the maximum in the appropriate direction.
        velocitySetpoint = (velocitySetpoint >= 0) ? MAX_VELOCITY : -MAX_VELOCITY;

// Drive
#ifdef SENDTRITIUM_MVP_PRINT_MES
        dumpInfo();
#endif
#ifndef SENDTRITIUM_MVP_EXPOSE_VARS
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

static void assertSendTritiumMVPError(SendTritium_MVP_error_code_t stmvperr) 
{
    Error_SendTritium_MVP = (error_code_t) stmvperr;

    switch(stmvperr)
    {
        case SENDTRITIUM_MVP_ERR_NONE:
            break;
        case SENDTRITIUM_MVP_ERR_GEAR_FAULT:
            // Assert a nonrecoverable error that will kill the motor, turn off contactors, display a fault screen, & infinite loop
            throwTaskError(Error_SendTritium_MVP, NULL, OPT_LOCK_SCHED, OPT_NONRECOV);
    }

    Error_SendTritium_MVP = SENDTRITIUM_MVP_ERR_NONE;
}
