/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file SendTritium_MVP_Cruise.c
 * @brief Function implementations for the SendTritium application MVP with cruise control
 * abilities.
 *
 * This contains functions relevant to updating the velocity and current setpoints
 * of the Tritium motor controller. The implementation includes a normal current
 * controlled mode and a cruise control mode, but doesn't include a one pedal mode or 
 * regenerative braking capabilities. The logic is determined through a finite state 
 * machine implementation.
 *
 * If the macro SENDTRITIUM_MVP_CRUISE_EXPOSE_VARS is defined prior to including 
 * SendTritium_MVP_Cruise.h, relevant setters will be exposed as externs for unit testing
 * and hardware inputs won't be read and motor commands won't be sent over MotorCAN.
 * If the macro SENDTRITIUM_MVP_CRUISE_PRINT_MES is also defined prior to including 
 * SendTritium_MVP_Cruise.h, debug info will be printed via UART.
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

#include "SendTritium_MVP_Cruise.h"



// Inputs
static bool cruiseEnable = false;
static bool cruiseSet = false;

static uint8_t brakePedalPercent = 0;
static uint8_t accelPedalPercent = 0;

static Gear_t gear = PARK_GEAR;

// Outputs
static float currentSetpoint = 0.0f;
static float velocitySetpoint = 0.0f;
static float cruiseVelSetpoint = 0.0f;
static float busCurrentSetPoint = 69.0f; // why is this float and not int?

// Current observed velocity
static float velocityObserved = 0.0f;

// Counter for sending setpoints to motor
static uint8_t motorMsgCounter = 0;

// Debouncing counters
static uint8_t cruiseEnableCounter = 0;
static uint8_t cruiseSetCounter = 0;

// Button states
static bool cruiseEnableButton = false;
static bool cruiseEnablePrevious = false;
static bool cruiseSetButton = false;
static bool cruiseSetPrevious = false;

// Allows cruiseSet to toggle on for a single loop of the SendTritium_MVP_Cruise task
static bool cruiseSetAllow = true;

// Accel pedal states (used only for hysteresis)
static bool accelPressed = false;

// FSM
static TritiumState_t prevState; // Previous state
static TritiumState_t state;     // Current state

// Getter functions for local variables in SendTritium_MVP_Cruise.c
GETTER(bool, cruiseEnable)
GETTER(bool, cruiseSet)
GETTER(uint8_t, brakePedalPercent)
GETTER(uint8_t, accelPedalPercent)
GETTER(Gear_t, gear)
TritiumStateName_t get_state(void) {return state.name;}
GETTER(float, velocityObserved)
GETTER(float, cruiseVelSetpoint)
GETTER(float, currentSetpoint)
GETTER(float, velocitySetpoint)

// Setter functions for local variables in SendTritium_MVP_Cruise.c
#ifdef SENDTRITIUM_MVP_CRUISE_EXPOSE_VARS
SETTER(bool, cruiseEnable)
SETTER(bool, cruiseSet)
SETTER(uint8_t, brakePedalPercent)
SETTER(uint8_t, accelPedalPercent)
SETTER(Gear_t, gear)
void set_state(TritiumStateName_t stateName) {state = FSM[stateName];}
SETTER(float, velocityObserved)
SETTER(float, cruiseVelSetpoint)
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
static void PoweredCruiseHandler(void);
static void PoweredCruiseDecider(void);
static void CoastingCruiseHandler(void);
static void CoastingCruiseDecider(void);
static void AccelerateCruiseHandler(void);
static void AccelerateCruiseDecider(void);

// Caller functions for state handlers & deciders in SendTritium_MVP_Cruise.c
#ifdef SENDTRITIUM_MVP_CRUISE_EXPOSE_VARS
void callForwardDriveHandler(void)      {ForwardDriveHandler();}
void callForwardDriveDecider(void)      {ForwardDriveDecider();}
void callParkHandler(void)              {ParkHandler();}
void callParkDecider(void)              {ParkDecider();}
void callReverseDriveHandler(void)      {ReverseDriveHandler();}
void callReverseDriveDecider(void)      {ReverseDriveDecider();}
void callPoweredCruiseHandler(void)     {PoweredCruiseHandler();}
void callPoweredCruiseDecider(void)     {PoweredCruiseDecider();}
void callCoastingCruiseHandler(void)    {CoastingCruiseHandler();}
void callCoastingCruiseDecider(void)    {CoastingCruiseDecider();}
void callAccelerateCruiseHandler(void)  {AccelerateCruiseHandler();}
void callAccelerateCruiseDecider(void)  {AccelerateCruiseDecider();}
#endif

// Function prototypes
static void assertSendTritiumMVPCruiseError(SendTritium_MVP_Cruise_error_code_t stmvpcerr);

// FSM
static const TritiumState_t FSM[6] = {
    {FORWARD_DRIVE, &ForwardDriveHandler, &ForwardDriveDecider},
    {PARK_STATE, &ParkHandler, &ParkDecider},
    {REVERSE_DRIVE, &ReverseDriveHandler, &ReverseDriveDecider},
    {POWERED_CRUISE, &PoweredCruiseHandler, &PoweredCruiseDecider},
    {COASTING_CRUISE, &CoastingCruiseHandler, &CoastingCruiseDecider},
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

/**
 * @brief Updates the brakelight and brake indication on display
 */
static void brakeUpdate(){
    if(brakePedalPercent >= BRAKE_PEDAL_THRESHOLD) {
        Minions_Write(BRAKELIGHT, true);
        UpdateDisplay_SetBrake(true);
    } else {
        Minions_Write(BRAKELIGHT, false);
        UpdateDisplay_SetBrake(false);
    }
}

#ifdef SENDTRITIUM_MVP_CRUISE_PRINT_MES
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
        strcpy(nameStr, "PARK_STATE");
        break;
    case REVERSE_DRIVE:
        strcpy(nameStr, "REVERSE_DRIVE");
        break;
    case POWERED_CRUISE:
        strcpy(nameStr, "POWERED_CRUISE");
        break;
    case COASTING_CRUISE:
        strcpy(nameStr, "COASTING_CRUISE");
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
    printf("brakePedalPercent: %d\n\r", brakePedalPercent);
    printf("accelPedalPercent: %d\n\r", accelPedalPercent);
    printf("gear: %d\n\r", (uint8_t)gear);
    print_float("currentSetpoint: ", currentSetpoint);
    print_float("velocitySetpoint: ", velocitySetpoint);
    print_float("velocityObserved: ", velocityObserved);
    printf("-------------------\n\r");
}
#endif

#ifndef SENDTRITIUM_MVP_CRUISE_EXPOSE_VARS
/**
 * @brief Reads inputs from the system
 */
static void readInputs()
{

    // Update pedals
    static uint32_t brakeSaturationCt = 0;
    static uint32_t accelSaturationCt = 0;

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
    // Used for accel hysteresis to prevent abrupt switches to/from ACCELERATE_CRUISE state
    if(accelSaturationCt < 3) {
        if(accelPedalPercent >= ACCEL_PEDAL_PRESSED_THRESHOLD)
            accelSaturationCt++;
        else if (accelSaturationCt != 0)
            accelSaturationCt--;

        if(accelSaturationCt == 0)
            accelPressed = false;
    }
    else if (accelSaturationCt >= 3) {
        if(accelPedalPercent <= ACCEL_PEDAL_UNPRESSED_THRESHOLD)
            accelSaturationCt--;
        
        accelPressed = true;
    }

    // Protect from both brake and accel being pressed
    if (brakePedalPercent == 100 && accelPedalPercent == 100) accelPedalPercent = 0;

    // Update buttons
    if(Minions_Read(CRUZ_EN)) { 
        if(cruiseEnableCounter < DEBOUNCE_PERIOD) cruiseEnableCounter++;
    }
    else {
        if(cruiseEnableCounter > 0) cruiseEnableCounter--;
    }
    if(Minions_Read(CRUZ_ST)) {
        if(cruiseSetCounter < DEBOUNCE_PERIOD) cruiseSetCounter++;
    }
    else {
        if(cruiseSetCounter > 0) cruiseSetCounter--;
    } 

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
            assertSendTritiumMVPCruiseError(SENDTRITIUM_MVP_CRUISE_ERR_GEAR_FAULT);
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
    else if (state.name == FORWARD_DRIVE || POWERED_CRUISE || COASTING_CRUISE || ACCELERATE_CRUISE)
        UpdateDisplay_SetGear(DISP_FORWARD);
    else if (state.name == REVERSE_DRIVE)
        UpdateDisplay_SetGear(DISP_REVERSE);

    // Debouncing
    cruiseEnableButton = false;
    cruiseSetButton = false;
    if(cruiseEnableCounter == DEBOUNCE_PERIOD){cruiseEnableButton = true;}
    else if(cruiseEnableCounter == 0){cruiseEnableButton = false;}

    if(cruiseSetCounter == DEBOUNCE_PERIOD){cruiseSetButton = true;}
    else if(cruiseSetCounter == 0){cruiseSetButton = false; cruiseSetAllow = true;}

    // Toggle CRUZ_EN
    if(cruiseEnableButton != cruiseEnablePrevious && cruiseEnablePrevious) // Falling edge toggle/CRUZ_EN detection
    {
        cruiseEnable = !cruiseEnable;
    }
    cruiseEnablePrevious = cruiseEnableButton;
    // cruiseEnablePrevious = false;

    // Allow CRUZ_SET on for just one loop of the SendTritium_MVP_Cruise task
    if(cruiseSet) 
    {
        cruiseSet = false;
        cruiseSetAllow = false;
    }
    if (!cruiseSetPrevious && cruiseSetButton && cruiseSetAllow) 
    {
        cruiseSet = true;
    }
    cruiseSetPrevious = cruiseSetButton;

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
    }

    // If braking, set current to 0. Otherwise, set current based on accel
    if(brakePedalPercent >= BRAKE_PEDAL_THRESHOLD) {
        velocitySetpoint = MAX_VELOCITY;
        currentSetpoint = 0.0f;
    } else {
        velocitySetpoint = MAX_VELOCITY;
        currentSetpoint = percentToFloat(map(accelPedalPercent, ACCEL_PEDAL_THRESHOLD, PEDAL_MAX, CURRENT_SP_MIN, CURRENT_SP_MAX));
    }

    // Turn brakelight on/off
    brakeUpdate();

    cruiseEnable = false; // Doesn't affect operation of cruiseEnable, as readInputs() happens after stateHandler
}

/**
 * @brief Forward Drive State Decider. Determines transitions out of
 * forward drive state (park, powered cruise).
 */
static void ForwardDriveDecider()
{
    // Go to PARK_STATE if you're in another gear
    if (gear == PARK_GEAR || gear == REVERSE_GEAR)
    {
        state = FSM[PARK_STATE];
    }
    else if (cruiseEnable && cruiseSet && velocityObserved >= MIN_CRUISE_VELOCITY) {
        state = FSM[POWERED_CRUISE];
        cruiseVelSetpoint = velocityObserved;
    }
    // Otherwise, stays in FORWARD_DRIVE
}

/**
 * @brief Park State Handler. No current is sent to the motor.
 */
static void ParkHandler()
{
    if (prevState.name != state.name)
    {
        UpdateDisplay_SetCruiseState(DISP_DISABLED);
    }
    velocitySetpoint = MAX_VELOCITY;
    currentSetpoint = 0.0f;

    // Turn brakelight on/off
    brakeUpdate();

    cruiseEnable = false; // Doesn't affect operation of cruiseEnable, as readInputs() happens after stateHandler
}

/**
 * @brief Park State Decider. Determines transitions out of
 * neutral drive state (forward drive, reverse drive).
 */
static void ParkDecider()
{
    // Switch for FORWARD_DRIVE/REVERSE_DRIVE if in FORWARD_GEAR/REVERSE_GEAR & speed is less than MAX_GEARSWITCH_VELOCITY
    if (gear == FORWARD_GEAR && (velocityObserved < 0 ? (velocityObserved >= -MAX_GEARSWITCH_VELOCITY) : (velocityObserved <= MAX_GEARSWITCH_VELOCITY)))
    {
        state = FSM[FORWARD_DRIVE];
    }
    else if (gear == REVERSE_GEAR && (velocityObserved < 0 ? (velocityObserved >= -MAX_GEARSWITCH_VELOCITY) : (velocityObserved <=MAX_GEARSWITCH_VELOCITY)))
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
    brakeUpdate();

    cruiseEnable = false; // Doesn't affect operation of cruiseEnable, as readInputs() happens after stateHandler
}

/**
 * @brief Reverse Drive State Decider. Determines transitions out of
 * reverse drive state (brake).
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

/**
 * @brief Powered Cruise State. Continue to travel at the recorded velocity as long as
 * Observed Velocity <= Velocity Setpoint
 */
static void PoweredCruiseHandler()
{
    if (prevState.name == FORWARD_DRIVE)
    {
        UpdateDisplay_SetCruiseState(DISP_ACTIVE);
    }
    velocitySetpoint = cruiseVelSetpoint;
    currentSetpoint = 1.0f;

    // Turn brakelight on/off
    brakeUpdate();

    // Set new cruiseVelSetpoint if cruise set is pressed
    if (cruiseSet && velocityObserved >= MIN_CRUISE_VELOCITY)
    {
        cruiseVelSetpoint = velocityObserved;
    }
}

/**
 * @brief Powered Cruise State Decider. Determines transitions out of powered
 * cruise state (park, forward drive, accelerate cruise, 
 * coasting cruise).
 */
static void PoweredCruiseDecider()
{
    // If you're no longer in FORWARD_GEAR or you're braking, exit cruise
    if (brakePedalPercent >= BRAKE_PEDAL_THRESHOLD || gear == PARK_STATE || gear == REVERSE_GEAR)
    {
        state = FSM[PARK_STATE];
    }
    // If cruise has been disabled, return to forward drive
    else if (!cruiseEnable)
    {
        state = FSM[FORWARD_DRIVE];
    }
    else if (accelPressed)
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
    currentSetpoint = 0.0f;

    // Turn brakelight on/off
    brakeUpdate();

    // Set new cruiseVelSetpoint if cruise set is pressed
    if (cruiseSet && velocityObserved >= MIN_CRUISE_VELOCITY)
    {
        cruiseVelSetpoint = velocityObserved;
    }
}

/**
 * @brief Coasting Cruise State Decider. Determines transitions out of coasting
 * cruise state (brake, neutral drive, one pedal, forward drive, record velocity,
 * accelerate cruise, powered cruise).
 */
static void CoastingCruiseDecider()
{
    // If you're no longer in FORWARD_GEAR or you're braking, exit cruise
    if (brakePedalPercent >= BRAKE_PEDAL_THRESHOLD || gear == PARK_STATE || gear == REVERSE_GEAR)
    {
        state = FSM[PARK_STATE];
    }
    // If cruise has been disabled, return to forward drive
    else if (!cruiseEnable)
    {
        state = FSM[FORWARD_DRIVE];
    }
    else if (accelPressed)
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

    // Turn brakelight on/off
    brakeUpdate();

    // Set new cruiseVelSetpoint if cruise set is pressed
    if (cruiseSet && velocityObserved >= MIN_CRUISE_VELOCITY)
    {
        cruiseVelSetpoint = velocityObserved;
    }
}

/**
 * @brief Accelerate Cruise State Decider. Determines transitions out of accelerate
 * cruise state (brake, neutral drive, one pedal, forward drive, record velocity,
 * coasting cruise).
 */
static void AccelerateCruiseDecider()
{
    // If you're no longer in FORWARD_GEAR or you're braking, exit cruise
    if (brakePedalPercent >= BRAKE_PEDAL_THRESHOLD || gear == PARK_STATE || gear == REVERSE_GEAR)
    {
        state = FSM[PARK_STATE];
    }
    // If cruise has been disabled, return to forward drive
    else if (!cruiseEnable)
    {
        state = FSM[FORWARD_DRIVE];
    }
    else if (!accelPressed)
    {
        state = FSM[COASTING_CRUISE];
    }
}

// Task (main loop)

/**
 * @brief Follows the FSM to update the velocity of the car
 */
void Task_SendTritium_MVP_Cruise(void *p_arg)
{
    OS_ERR err;

    // Initialize current state to PARK_STATE
    state = FSM[PARK_STATE];
    prevState = FSM[PARK_STATE];
    UpdateDisplay_SetGear(PARK_GEAR);

    // Initialize Regen & Cruise disabled
    // NOTE: Regen stays disabled on Daybreak MVP + Cruise
    UpdateDisplay_SetRegenState(DISP_DISABLED);
    UpdateDisplay_SetCruiseState(DISP_DISABLED);

#ifndef SENDTRITIUM_MVP_CRUISE_EXPOSE_VARS
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
#ifndef SENDTRITIUM_MVP_CRUISE_EXPOSE_VARS
        readInputs(); // read inputs from the system
        UpdateDisplay_SetAccel(accelPedalPercent);
#endif
        prevState = state;
        state.stateDecider(); // decide what the next state is

        // This ONLY works for MVP (since Cruise has velocity control)
        // // Disable velocity controlled mode by always overwriting velocity to the maximum
        // // in the appropriate direction.
        // velocitySetpoint = (velocitySetpoint > 0) ? MAX_VELOCITY : -MAX_VELOCITY;

// Drive
#ifdef SENDTRITIUM_MVP_CRUISE_PRINT_MES
        dumpInfo();
#endif
#ifndef SENDTRITIUM_MVP_CRUISE_EXPOSE_VARS
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

static void assertSendTritiumMVPCruiseError(SendTritium_MVP_Cruise_error_code_t stmvpcerr)
{
    Error_SendTritium_MVP_Cruise = (error_code_t) stmvpcerr;

    switch(stmvpcerr)
    {
        case SENDTRITIUM_MVP_CRUISE_ERR_NONE:
            break;
        case SENDTRITIUM_MVP_CRUISE_ERR_GEAR_FAULT:
            // Assert a nonrecoverable error that will kill the motor, turn off contactors, display a fault screen, & infinite loop
            throwTaskError(Error_SendTritium_MVP_Cruise, NULL, OPT_LOCK_SCHED, OPT_NONRECOV);
    }

    Error_SendTritium_MVP_Cruise = SENDTRITIUM_MVP_CRUISE_ERR_NONE;
}