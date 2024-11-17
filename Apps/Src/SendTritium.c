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
#include "SendTritium.h"
#include "ReadTritium.h"
#include "SendCarCAN.h"
#include "CANbus.h"
#include "UpdateDisplay.h"
#include "CANConfig.h"
#include "common.h"

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
static bool cruiseEnable = false;
static bool cruiseSet = false;
static bool onePedalEnable = false;
static bool regenEnable = false;

static uint8_t brakePedalPercent = 0;
static uint8_t accelPedalPercent = 0;

static Gear_t gear = NEUTRAL_GEAR;

// Outputs
float currentSetpoint = 0;
float velocitySetpoint = 0;
float cruiseVelSetpoint = 0;
float busCurrentSetPoint = 69.0f; // why is this float and not int?

// Current observed velocity
static float velocityObserved = 0;

// Counter for sending setpoints to motor
static uint8_t motorMsgCounter = 0;

// Debouncing counters
// static uint8_t onePedalCounter = 0;
// static uint8_t cruiseEnableCounter = 0;
// static uint8_t cruiseSetCounter = 0;

// Button states
static bool onePedalButton = false;
static bool onePedalPrevious = false;

static bool cruiseEnableButton = false;
static bool cruiseEnablePrevious = false;

// FSM
static TritiumState_t prevState; // Previous state
static TritiumState_t state;     // Current state

// Getter functions for local variables in SendTritium.c
GETTER(bool, cruiseEnable)
GETTER(bool, cruiseSet)
GETTER(bool, onePedalEnable)
GETTER(bool, regenEnable)
GETTER(uint8_t, brakePedalPercent)
GETTER(uint8_t, accelPedalPercent)
GETTER(Gear_t, gear)
GETTER(TritiumState_t, state)
GETTER(float, velocityObserved)
GETTER(float, cruiseVelSetpoint)
GETTER(float, currentSetpoint)
GETTER(float, velocitySetpoint)

// Setter functions for local variables in SendTritium.c
#ifdef SENDTRITIUM_EXPOSE_VARS
SETTER(bool, cruiseEnable)
SETTER(bool, cruiseSet)
SETTER(bool, onePedalEnable)
SETTER(bool, regenEnable)
SETTER(uint8_t, brakePedalPercent)
SETTER(uint8_t, accelPedalPercent)
SETTER(Gear_t, gear)
SETTER(TritiumState_t, state)
SETTER(float, velocityObserved)
SETTER(float, cruiseVelSetpoint)
SETTER(float, currentSetpoint)
SETTER(float, velocitySetpoint)
#endif

// Handler & Decider Declarations
static void ForwardDriveHandler(void);
static void ForwardDriveDecider(void);
static void NeutralDriveHandler(void);
static void NeutralDriveDecider(void);
static void ReverseDriveHandler(void);
static void ReverseDriveDecider(void);
static void RecordVelocityHandler(void);
static void RecordVelocityDecider(void);
static void PoweredCruiseHandler(void);
static void PoweredCruiseDecider(void);
static void CoastingCruiseHandler(void);
static void CoastingCruiseDecider(void);
static void BrakeHandler(void);
static void BrakeDecider(void);
static void OnePedalDriveHandler(void);
static void OnePedalDriveDecider(void);
static void AccelerateCruiseHandler(void);
static void AccelerateCruiseDecider(void);

// FSM
static const TritiumState_t FSM[9] = {
    {FORWARD_DRIVE, &ForwardDriveHandler, &ForwardDriveDecider},
    {NEUTRAL_DRIVE, &NeutralDriveHandler, &NeutralDriveDecider},
    {REVERSE_DRIVE, &ReverseDriveHandler, &ReverseDriveDecider},
    {RECORD_VELOCITY, &RecordVelocityHandler, &RecordVelocityDecider},
    {POWERED_CRUISE, &PoweredCruiseHandler, &PoweredCruiseDecider},
    {COASTING_CRUISE, &CoastingCruiseHandler, &CoastingCruiseDecider},
    {BRAKE_STATE, &BrakeHandler, &BrakeDecider},
    {ONEPEDAL, &OnePedalDriveHandler, &OnePedalDriveDecider},
    {ACCELERATE_CRUISE, &AccelerateCruiseHandler, &AccelerateCruiseDecider}};

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
    case NEUTRAL_DRIVE:
        strcpy(nameStr, "NEUTRAL_DRIVE");
        break;
    case REVERSE_DRIVE:
        strcpy(nameStr, "REVERSE_DRIVE");
        break;
    case RECORD_VELOCITY:
        strcpy(nameStr, "RECORD_VELOCITY");
        break;
    case POWERED_CRUISE:
        strcpy(nameStr, "POWERED_CRUISE");
        break;
    case COASTING_CRUISE:
        strcpy(nameStr, "COASTING_CRUISE");
        break;
    case BRAKE_STATE:
        strcpy(nameStr, "BRAKE_STATE");
        break;
    case ONEPEDAL:
        strcpy(nameStr, "ONEPEDAL");
        break;
    case ACCELERATE_CRUISE:
        strcpy(nameStr, "ACCELERATE_CRUISE");
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
    printf("cruiseEnable: %d\n\r", cruiseEnable);
    printf("cruiseSet: %d\n\r", cruiseSet);
    printf("onePedalEnable: %d\n\r", onePedalEnable);
    printf("brakePedalPercent: %d\n\r", brakePedalPercent);
    printf("accelPedalPercent: %d\n\r", accelPedalPercent);
    printf("gear: %d\n\r", (uint8_t)gear);
    print_float("currentSetpoint: ", currentSetpoint);
    print_float("velocitySetpoint: ", velocitySetpoint);
    print_float("velocityObserved: ", velocityObserved);
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

    // Update regen enable
    // regenEnable = ChargeEnable_Get();
    regenEnable = false;

    // Update buttons
    // if(Minions_Read(REGEN_SW) && onePedalCounter < DEBOUNCE_PERIOD){onePedalCounter++;}
    // else if(onePedalCounter > 0){onePedalCounter--;}

    // if(Minions_Read(CRUZ_EN) && cruiseEnableCounter < DEBOUNCE_PERIOD){cruiseEnableCounter++;}
    // else if(cruiseEnableCounter > 0){cruiseEnableCounter--;}

    // if(Minions_Read(CRUZ_ST) && cruiseSetCounter < DEBOUNCE_PERIOD){cruiseSetCounter++;}
    // else if(cruiseSetCounter > 0){cruiseSetCounter--;}

    // Update gears
    bool forwardSwitch = Minions_Read(FOR_SW);
    bool reverseSwitch = Minions_Read(REV_SW);
    bool forwardGear = (forwardSwitch && !reverseSwitch);
    bool reverseGear = (!forwardSwitch && reverseSwitch);
    bool neutralGear = (!forwardSwitch && !reverseSwitch);

    uint8_t gearFault = (uint8_t)forwardGear + (uint8_t)reverseGear + (uint8_t)neutralGear;
    static uint8_t gearFaultCnt = 0;

    if (gearFault != 1)
    {
        // Fault behavior
        if (gearFaultCnt > GEAR_FAULT_THRESHOLD)
            state = FSM[NEUTRAL_DRIVE];
        else
            gearFaultCnt++;
    }
    else
    {
        gearFaultCnt = 0;
    }

    if (neutralGear)
        gear = NEUTRAL_GEAR;
    else if (forwardGear)
        gear = FORWARD_GEAR;
    else if (reverseGear)
        gear = REVERSE_GEAR;
    else
        gear = NEUTRAL_GEAR;

    if (gear == NEUTRAL_GEAR)
        UpdateDisplay_SetGear(DISP_NEUTRAL);
    else if (gear == FORWARD_GEAR)
        UpdateDisplay_SetGear(DISP_FORWARD);
    else if (gear == REVERSE_GEAR)
        UpdateDisplay_SetGear(DISP_REVERSE);

    // Debouncing
    onePedalButton = false;
    cruiseEnableButton = false;
    cruiseSet = false;
    // if(onePedalCounter == DEBOUNCE_PERIOD){onePedalButton = true;}
    // else if(onePedalCounter == 0){onePedalButton = false;}

    // if(cruiseEnableCounter == DEBOUNCE_PERIOD){cruiseEnableButton = true;}
    // else if(cruiseEnableCounter == 0){cruiseEnableButton = false;}

    // if(cruiseSetCounter == DEBOUNCE_PERIOD){cruiseSet = true;}
    // else if(cruiseSetCounter == 0){cruiseSet = false;}

    // Toggle
    // if(onePedalButton != onePedalPrevious && onePedalPrevious){onePedalEnable = !onePedalEnable;}
    // if(!regenEnable) onePedalEnable = false;
    // onePedalPrevious = onePedalButton;
    onePedalEnable = false;
    onePedalPrevious = false;

    // if(cruiseEnableButton != cruiseEnablePrevious && cruiseEnablePrevious){cruiseEnable = !cruiseEnable;}
    // cruiseEnablePrevious = cruiseEnableButton;
    cruiseEnablePrevious = false;

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

// State Handlers & Deciders

/**
 * @brief Forward Drive State Handler. Accelerator is mapped directly
 * to current setpoint at positive velocity.
 */
static void ForwardDriveHandler()
{
    if (prevState.name != state.name)
    {
        UpdateDisplay_SetCruiseState(DISP_DISABLED);
        UpdateDisplay_SetRegenState(DISP_DISABLED);
    }
    velocitySetpoint = MAX_VELOCITY;
    // printf("accelPedalPercent: %d\n\r", accelPedalPercent);
    uint8_t after_map = map(accelPedalPercent, ACCEL_PEDAL_THRESHOLD, PEDAL_MAX, CURRENT_SP_MIN, CURRENT_SP_MAX);
    // printf("map(accelPedalPercent): %d\n\r", after_map);
    currentSetpoint = percentToFloat(after_map);
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
        state = FSM[BRAKE_STATE];
    }
    else if (cruiseSet && cruiseEnable && velocityObserved >= MIN_CRUISE_VELOCITY)
    {
        state = FSM[RECORD_VELOCITY];
    }
    else if (onePedalEnable)
    {
        state = FSM[ONEPEDAL];
    }
    else if (gear == NEUTRAL_GEAR || gear == REVERSE_GEAR)
    {
        state = FSM[NEUTRAL_DRIVE];
    }
}

/**
 * @brief Neutral Drive State Handler. No current is sent to the motor.
 */
static void NeutralDriveHandler()
{
    if (prevState.name != state.name)
    {
        UpdateDisplay_SetCruiseState(DISP_DISABLED);
        UpdateDisplay_SetRegenState(DISP_DISABLED);
    }
    velocitySetpoint = MAX_VELOCITY;
    currentSetpoint = 0.0f;

    cruiseEnable = false;
    onePedalEnable = false;
}

/**
 * @brief Neutral Drive State Decider. Determines transitions out of
 * neutral drive state (brake, forward drive, reverse drive).
 */
static void NeutralDriveDecider()
{
    if (brakePedalPercent >= BRAKE_PEDAL_THRESHOLD)
    {
        state = FSM[BRAKE_STATE];
    }
    else if (gear == FORWARD_GEAR)
    {
        state = FSM[FORWARD_DRIVE];
    }
    else if (gear == REVERSE_GEAR)
    {
        state = FSM[REVERSE_DRIVE];
    }
}

/**
 * @brief Reverse Drive State Handler. Accelerator is mapped directly to
 * current setpoint (at negative velocity).
 */
static void ReverseDriveHandler()
{
    if (prevState.name != state.name)
    {
        UpdateDisplay_SetCruiseState(DISP_DISABLED);
        UpdateDisplay_SetRegenState(DISP_DISABLED);
    }
    velocitySetpoint = -MAX_VELOCITY;
    currentSetpoint = percentToFloat(map(accelPedalPercent, ACCEL_PEDAL_THRESHOLD, PEDAL_MAX, CURRENT_SP_MIN, CURRENT_SP_MAX));
    cruiseEnable = false;
    onePedalEnable = false;
}

/**
 * @brief Reverse Drive State Decider. Determines transitions out of
 * reverse drive state (brake, neutral drive).
 */
static void ReverseDriveDecider()
{
    if (brakePedalPercent >= BRAKE_PEDAL_THRESHOLD)
    {
        state = FSM[BRAKE_STATE];
    }
    else if (gear == NEUTRAL_GEAR || gear == FORWARD_GEAR)
    {
        state = FSM[NEUTRAL_DRIVE];
    }
}

/**
 * @brief Record Velocity State. While pressing the cruise set button,
 * the car will record the observed velocity into velocitySetpoint.
 */
static void RecordVelocityHandler()
{
    if (prevState.name != state.name)
    {
        UpdateDisplay_SetCruiseState(DISP_ACTIVE);
        UpdateDisplay_SetRegenState(DISP_DISABLED);
    }
    // put car in neutral while recording velocity (while button is held)
    velocitySetpoint = MAX_VELOCITY;
    currentSetpoint = 0;
    cruiseVelSetpoint = velocityObserved;
}

/**
 * @brief Record Velocity State Decider. Determines transitions out of record velocity
 * state (brake, neutral drive, one pedal, forward drive, powered cruise).
 */
static void RecordVelocityDecider()
{
    if (brakePedalPercent >= BRAKE_PEDAL_THRESHOLD)
    {
        state = FSM[BRAKE_STATE];
    }
    else if (gear == NEUTRAL_GEAR || gear == REVERSE_GEAR)
    {
        state = FSM[NEUTRAL_DRIVE];
    }
    else if (onePedalEnable)
    {
        cruiseEnable = false;
        state = FSM[ONEPEDAL];
    }
    else if (!cruiseEnable)
    {
        state = FSM[FORWARD_DRIVE];
    }
    else if (cruiseEnable && !cruiseSet)
    {
        state = FSM[POWERED_CRUISE];
    }
}

/**
 * @brief Powered Cruise State. Continue to travel at the recorded velocity as long as
 * Observed Velocity <= Velocity Setpoint
 */
static void PoweredCruiseHandler()
{
    velocitySetpoint = cruiseVelSetpoint;
    currentSetpoint = 1.0f;
}

/**
 * @brief Powered Cruise State Decider. Determines transitions out of powered
 * cruise state (brake, neutral drive, one pedal, forward drive, record velocity,
 * accelerate cruise, coasting cruise).
 */
static void PoweredCruiseDecider()
{
    if (brakePedalPercent >= BRAKE_PEDAL_THRESHOLD)
    {
        state = FSM[BRAKE_STATE];
    }
    else if (gear == NEUTRAL_GEAR || gear == REVERSE_GEAR)
    {
        state = FSM[NEUTRAL_DRIVE];
    }
    else if (onePedalEnable)
    {
        cruiseEnable = false;
        state = FSM[ONEPEDAL];
    }
    else if (!cruiseEnable)
    {
        state = FSM[FORWARD_DRIVE];
    }
    else if (cruiseSet && velocityObserved >= MIN_CRUISE_VELOCITY)
    {
        state = FSM[RECORD_VELOCITY];
    }
    else if (accelPedalPercent >= ACCEL_PEDAL_THRESHOLD)
    {
        state = FSM[ACCELERATE_CRUISE];
    }
    else if (velocityObserved > cruiseVelSetpoint)
    {
        state = FSM[COASTING_CRUISE];
    }
}

/**
 * @brief Coasting Cruise State. We do not want to utilize motor braking
 * in cruise control mode due to safety issues. Coast the motor (go into neutral)
 * if we want to slow down.
 */
static void CoastingCruiseHandler()
{
    velocitySetpoint = cruiseVelSetpoint;
    currentSetpoint = 0;
}

/**
 * @brief Coasting Cruise State Decider. Determines transitions out of coasting
 * cruise state (brake, neutral drive, one pedal, forward drive, record velocity,
 * accelerate cruise, powered cruise).
 */
static void CoastingCruiseDecider()
{
    if (brakePedalPercent >= BRAKE_PEDAL_THRESHOLD)
    {
        state = FSM[BRAKE_STATE];
    }
    else if (gear == NEUTRAL_GEAR || gear == REVERSE_GEAR)
    {
        state = FSM[NEUTRAL_DRIVE];
    }
    else if (onePedalEnable)
    {
        cruiseEnable = false;
        state = FSM[ONEPEDAL];
    }
    else if (!cruiseEnable)
    {
        state = FSM[FORWARD_DRIVE];
    }
    else if (cruiseSet && velocityObserved >= MIN_CRUISE_VELOCITY)
    {
        state = FSM[RECORD_VELOCITY];
    }
    else if (accelPedalPercent >= ACCEL_PEDAL_THRESHOLD)
    {
        state = FSM[ACCELERATE_CRUISE];
    }
    else if (velocityObserved <= cruiseVelSetpoint)
    {
        state = FSM[POWERED_CRUISE];
    }
}

/**
 * @brief Accelerate Cruise State. In the event that the driver needs to accelerate in cruise
 * mode, we will accelerate to the pedal percentage. Upon release of the accelerator
 * pedal, we will return to cruise mode at the previously recorded velocity.
 */
static void AccelerateCruiseHandler()
{
    velocitySetpoint = MAX_VELOCITY;
    currentSetpoint = percentToFloat(map(accelPedalPercent, ACCEL_PEDAL_THRESHOLD, PEDAL_MAX, CURRENT_SP_MIN, CURRENT_SP_MAX));
}

/**
 * @brief Accelerate Cruise State Decider. Determines transitions out of accelerate
 * cruise state (brake, neutral drive, one pedal, forward drive, record velocity,
 * coasting cruise).
 */
static void AccelerateCruiseDecider()
{
    if (brakePedalPercent >= BRAKE_PEDAL_THRESHOLD)
    {
        state = FSM[BRAKE_STATE];
    }
    else if (gear == NEUTRAL_GEAR || gear == REVERSE_GEAR)
    {
        state = FSM[NEUTRAL_DRIVE];
    }
    else if (onePedalEnable)
    {
        cruiseEnable = false;
        state = FSM[ONEPEDAL];
    }
    else if (!cruiseEnable)
    {
        state = FSM[FORWARD_DRIVE];
    }
    else if (cruiseSet && velocityObserved >= MIN_CRUISE_VELOCITY)
    {
        state = FSM[RECORD_VELOCITY];
    }
    else if (accelPedalPercent < ACCEL_PEDAL_THRESHOLD)
    {
        state = FSM[COASTING_CRUISE];
    }
}

/**
 * @brief One Pedal Drive State. When in one pedal drive, if the accelerator percentage is lower
 * than ONEPEDAL_BRAKE_THRESHOLD, the car will utilize motor braking to slow down. If accelerator
 * percentage is in the neutral zone, the car will coast. If accelerator percentage is above
 * the NEUTRAL_THRESHOLD, the car will accelerate as normal.
 */
static void OnePedalDriveHandler()
{
    if (prevState.name != state.name)
    {
        UpdateDisplay_SetCruiseState(DISP_DISABLED);
    }
    if (accelPedalPercent <= ONEPEDAL_BRAKE_THRESHOLD)
    {
        // Regen brake: Map 0 -> brake to 100 -> 0
        velocitySetpoint = 0;
        currentSetpoint = percentToFloat(map(accelPedalPercent, PEDAL_MIN, ONEPEDAL_BRAKE_THRESHOLD, CURRENT_SP_MAX, CURRENT_SP_MIN));
        Minions_Write(BRAKELIGHT, true);
        UpdateDisplay_SetRegenState(DISP_ACTIVE);
    }
    else if (ONEPEDAL_BRAKE_THRESHOLD < accelPedalPercent && accelPedalPercent <= ONEPEDAL_NEUTRAL_THRESHOLD)
    {
        // Neutral: coast
        velocitySetpoint = MAX_VELOCITY;
        currentSetpoint = 0;
        Minions_Write(BRAKELIGHT, false);
        UpdateDisplay_SetRegenState(DISP_ENABLED);
    }
    else if (ONEPEDAL_NEUTRAL_THRESHOLD < accelPedalPercent)
    {
        // Accelerate: Map neutral -> 100 to 0 -> 100
        velocitySetpoint = MAX_VELOCITY;
        currentSetpoint = percentToFloat(map(accelPedalPercent, ONEPEDAL_NEUTRAL_THRESHOLD, PEDAL_MAX, CURRENT_SP_MIN, CURRENT_SP_MAX));
        Minions_Write(BRAKELIGHT, false);
        UpdateDisplay_SetRegenState(DISP_ENABLED);
    }
}

/**
 * @brief One Pedal Drive State Decider. Determines transitions out of one pedal
 * drive state (brake, record velocity, neutral drive).
 */
static void OnePedalDriveDecider()
{
    if (brakePedalPercent >= BRAKE_PEDAL_THRESHOLD)
    {
        state = FSM[BRAKE_STATE];
    }
    else if (cruiseSet && cruiseEnable && velocityObserved >= MIN_CRUISE_VELOCITY)
    {
        state = FSM[RECORD_VELOCITY];
        Minions_Write(BRAKELIGHT, false);
    }
    else if (gear == NEUTRAL_GEAR || gear == REVERSE_GEAR)
    {
        state = FSM[NEUTRAL_DRIVE];
        Minions_Write(BRAKELIGHT, false);
    }
}

/**
 * @brief Brake State. When brake pedal is pressed, physical brakes will be active.
 * Put motor in neutral to prevent motor braking while physical brakes are engaged.
 * Additionally, disable all cruise control and one pedal functionality.
 */
static void BrakeHandler()
{
    if (prevState.name != state.name)
    {
        UpdateDisplay_SetCruiseState(DISP_DISABLED);
        UpdateDisplay_SetRegenState(DISP_DISABLED);
    }

    velocitySetpoint = MAX_VELOCITY;
    currentSetpoint = 0;
    cruiseEnable = false;
    onePedalEnable = false;

    Minions_Write(BRAKELIGHT, true);
    UpdateDisplay_SetBrake(true);
}

/**
 * @brief Brake State Decider. Determines transitions out of brake state (forward drive,
 * neutral drive).
 */
static void BrakeDecider()
{
    if (brakePedalPercent < BRAKE_PEDAL_THRESHOLD)
    {
        if (gear == FORWARD_GEAR)
        {
            state = FSM[FORWARD_DRIVE];
            Minions_Write(BRAKELIGHT, false);
            UpdateDisplay_SetBrake(false);
        }
        else if (gear == NEUTRAL_GEAR)
        {
            state = FSM[NEUTRAL_DRIVE];
            Minions_Write(BRAKELIGHT, false);
            UpdateDisplay_SetBrake(false);
        }
        else if (gear == REVERSE_GEAR)
        {
            state = FSM[REVERSE_DRIVE];
            Minions_Write(BRAKELIGHT, false);
            UpdateDisplay_SetBrake(false);
        }
    }
}

// Task (main loop)

/**
 * @brief Follows the FSM to update the velocity of the car
 */
void Task_SendTritium(void *p_arg)
{
    OS_ERR err;

    // Initialize current state to FORWARD_DRIVE
    state = FSM[NEUTRAL_DRIVE];
    prevState = FSM[NEUTRAL_DRIVE];

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
