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
#include "SendTritium.h"



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

// Allows cruiseSet to toggle on for a single loop of the SendTritium task
static bool cruiseSetAllow = true;

// Accel pedal states (used only for hysteresis)
static bool accelPressed = false;

// FSM
static TritiumState_t prevState; // Previous state
static TritiumState_t state;     // Current state

// Getter functions for local variables in SendTritium.c
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

// Setter functions for local variables in SendTritium.c
#ifdef SENDTRITIUM_EXPOSE_VARS
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
void ForwardDriveHandler(void);
void ForwardDriveDecider(void);
void ParkHandler(void);
void ParkDecider(void);
void ReverseDriveHandler(void);
void ReverseDriveDecider(void);
void PoweredCruiseHandler(void);
void PoweredCruiseDecider(void);
void CoastingCruiseHandler(void);
void CoastingCruiseDecider(void);
void AccelerateCruiseHandler(void);
void AccelerateCruiseDecider(void);

// Function prototypes
static void assertSendTritiumError(SendTritium_error_code_t sterr);

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
    if(brakePedalPercent == BRAKE_PRESSED) {
        Minions_Write(BRAKELIGHT, true);
        UpdateDisplay_SetBrake(true);
    } else {
        Minions_Write(BRAKELIGHT, false);
        UpdateDisplay_SetBrake(false);
    }
}

/**
 * @brief Sets new cruiseVelSetpoint if cruise set is pressed.
 * This function is only called while in the cruise states.
 */
static void cruiseVelCheckUpdate() {
    if (cruiseSet && velocityObserved >= MIN_CRUISE_VELOCITY)
    {
        cruiseVelSetpoint = velocityObserved;
    }
}

static bool bothPedalsPressed() {
    // Protect from both brake and accel being pressed (checks the smallest of ACCEL_PEDAL_PRESSED_THRESHOLD & 
    // ACCEL_PEDAL_THRESHOLD) to make sure we're never accelerating/sending current & also braking
    if (brakePedalPercent == BRAKE_PRESSED && accelPedalPercent >= ACCEL_BRAKE_THROTTLE_OVERRIDE_THRESHOLD)  {
        return true;
    }
    return false;
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
    printf("accelPedalPercent: %u\n\r", accelPedalPercent);
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
    static uint8_t brakeSaturationCt = 0;
    static uint8_t accelSaturationCt = 0;

    uint8_t latest_pedal = Pedals_Read(BRAKE);

    if (brakeSaturationCt < 3)
    {
        if (latest_pedal == 100) 
            {brakeSaturationCt++;}
        else if (latest_pedal == 0 && brakeSaturationCt != 0)
            {brakeSaturationCt--;}
        brakePedalPercent = BRAKE_UNPRESSED;
    }
    else if (brakeSaturationCt >= 3)
    {
        brakePedalPercent = BRAKE_PRESSED;

        if (latest_pedal == 0)
            {brakeSaturationCt = 0;}
    }

    accelPedalPercent = Pedals_Read(ACCELERATOR);
    // Used for accel hysteresis to prevent abrupt switches to/from ACCELERATE_CRUISE state
    if(accelSaturationCt < 3) {
        if(accelPedalPercent >= ACCEL_PEDAL_PRESSED_THRESHOLD)
            {accelSaturationCt++;}
        else if (accelSaturationCt != 0)
            {accelSaturationCt--;}

        if(accelSaturationCt == 0)
            {accelPressed = false;}
    }
    else if (accelSaturationCt >= 3) {
        if(accelPedalPercent <= ACCEL_PEDAL_UNPRESSED_THRESHOLD)
            {accelSaturationCt--;}
        
        accelPressed = true;
    }

    // Update buttons
    if(Minions_Read(CRUZ_EN)) { // Toggle button
        if(cruiseEnableCounter < DEBOUNCE_PERIOD) {cruiseEnableCounter++;}
    }
    else {
        if(cruiseEnableCounter > 0) {cruiseEnableCounter--;}
    }
    if(Minions_Read(CRUZ_ST)) { // When cruiseSetCounter reaches DEBOUNCE_PERIOD, cruiseSet will be on for one task 
                                // cycle (only after being completely off, as determined by cruiseSetAllow being previously off)
        if(cruiseSetCounter < DEBOUNCE_PERIOD) {cruiseSetCounter++;}
    }
    else {
        if(cruiseSetCounter > 0) {cruiseSetCounter--;}
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
            {assertSendTritiumError(SENDTRITIUM_ERR_GEAR_FAULT);}
        else
            {gearFaultCnt++;}
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

    // Display is state-based
    if (state.name == PARK_STATE)
    {
        UpdateDisplay_SetGear(DISP_PARK);
    }
    else if (state.name == FORWARD_DRIVE || state.name == POWERED_CRUISE || state.name == COASTING_CRUISE || state.name  == ACCELERATE_CRUISE)
    {
        UpdateDisplay_SetGear(DISP_FORWARD);
    }
    else if (state.name == REVERSE_DRIVE)
    {
        UpdateDisplay_SetGear(DISP_REVERSE);
    }

    // Debouncing
    cruiseEnableButton = false;
    cruiseSetButton = false;
    if(cruiseEnableCounter == DEBOUNCE_PERIOD){cruiseEnableButton = true;}
    else if(cruiseEnableCounter == 0){cruiseEnableButton = false;}

    if(cruiseSetCounter == DEBOUNCE_PERIOD){cruiseSetButton = true;}
    else if(cruiseSetCounter == 0){cruiseSetButton = false; cruiseSetAllow = true;} // Debounce & reset to allow cruise to be set again

    // Toggle CRUZ_EN
    if(cruiseEnableButton != cruiseEnablePrevious && cruiseEnablePrevious) // Falling edge toggle/CRUZ_EN detection
    {
        cruiseEnable = !cruiseEnable;
    }
    cruiseEnablePrevious = cruiseEnableButton;
    // cruiseEnablePrevious = false;

    // Allow CRUZ_SET on for just one loop of the SendTritium task
    if(cruiseSet) 
    {
        cruiseSet = false;
        cruiseSetAllow = false; // Can't set cruise again until it's recorded as unpressed
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
    {
        in_max = in_min; // The minimum of the input range should never be greater than the maximum of the input range
    }
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
void ForwardDriveHandler()
{
    if (prevState.name != state.name)
    {
        UpdateDisplay_SetCruiseState(DISP_DISABLED);
    }

    // If braking, set current to 0. Otherwise, set current based on accel
    if(brakePedalPercent == BRAKE_PRESSED || bothPedalsPressed()) {
        velocitySetpoint = MAX_VELOCITY;
        currentSetpoint = 0.0f;
    } else {
        velocitySetpoint = MAX_VELOCITY;
        currentSetpoint = percentToFloat(map(accelPedalPercent, ACCEL_PEDAL_THRESHOLD, PEDAL_MAX, CURRENT_SP_MIN, CURRENT_SP_MAX));
    }

    // Turn brakelight on/off
    brakeUpdate();
}

/**
 * @brief Forward Drive State Decider. Determines transitions out of
 * forward drive state (park, powered cruise).
 */
void ForwardDriveDecider()
{
    // Go to PARK_STATE if you're in another gear
    if (gear == PARK_GEAR || gear == REVERSE_GEAR)
    {
        state = FSM[PARK_STATE];
    }
    else if (cruiseEnable && cruiseSet && (velocityObserved >= MIN_CRUISE_VELOCITY) && (brakePedalPercent == BRAKE_UNPRESSED)) 
    {
        state = FSM[POWERED_CRUISE];
        cruiseVelSetpoint = velocityObserved;
    }
    // Otherwise, stays in FORWARD_DRIVE
}

/**
 * @brief Park State Handler. No current is sent to the motor.
 */
void ParkHandler()
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
void ParkDecider()
{
    if (gear == FORWARD_GEAR) // Protect from 
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
void ReverseDriveHandler()
{
    // If braking, set current to 0. Otherwise, set current based on accel
    if(brakePedalPercent == BRAKE_PRESSED || bothPedalsPressed()) {
        velocitySetpoint = MAX_VELOCITY;
        currentSetpoint = 0.0f;
    } else {
        velocitySetpoint = -MAX_VELOCITY;
        currentSetpoint = percentToFloat(map(accelPedalPercent, ACCEL_PEDAL_THRESHOLD, PEDAL_MAX, CURRENT_SP_MIN, CURRENT_SP_MAX));
    }

    // Turn brakelight on/off
    brakeUpdate();
}

/**
 * @brief Reverse Drive State Decider. Determines transitions out of
 * reverse drive state (brake).
 */
void ReverseDriveDecider()
{
    if (gear == PARK_GEAR || gear == FORWARD_GEAR)
    {
        state = FSM[PARK_STATE];
    }
}

/**
 * @brief Powered Cruise State. Continue to travel at the recorded velocity as long as
 * Observed Velocity <= Velocity Setpoint
 */
void PoweredCruiseHandler()
{
    if (prevState.name == FORWARD_DRIVE)
    {
        UpdateDisplay_SetCruiseState(DISP_ACTIVE);
    }
    velocitySetpoint = cruiseVelSetpoint;
    currentSetpoint = 1.0f;

    // Turn brakelight on/off
    brakeUpdate();

    // Check cruise set and update cruiseVelSetpoint accordingly
    cruiseVelCheckUpdate();
}

/**
 * @brief Powered Cruise State Decider. Determines transitions out of powered
 * cruise state (park, forward drive, accelerate cruise, 
 * coasting cruise).
 */
void PoweredCruiseDecider()
{
    // If you're no longer in FORWARD_GEAR or you're braking, exit cruise
    if (brakePedalPercent == BRAKE_PRESSED || gear == PARK_GEAR || gear == REVERSE_GEAR)
    {
        state = FSM[PARK_STATE];
        cruiseEnable = false;
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
void CoastingCruiseHandler()
{
    velocitySetpoint = cruiseVelSetpoint;
    currentSetpoint = 0.0f;

    // Turn brakelight on/off
    brakeUpdate();

    // Check cruise set and update cruiseVelSetpoint accordingly
    cruiseVelCheckUpdate();
}

/**
 * @brief Coasting Cruise State Decider. Determines transitions out of coasting
 * cruise state (brake, neutral drive, one pedal, forward drive, record velocity,
 * accelerate cruise, powered cruise).
 */
void CoastingCruiseDecider()
{
    // If you're no longer in FORWARD_GEAR or you're braking, exit cruise
    if (brakePedalPercent == BRAKE_PRESSED || gear == PARK_GEAR || gear == REVERSE_GEAR)
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
void AccelerateCruiseHandler()
{
    velocitySetpoint = MAX_VELOCITY;
    currentSetpoint = percentToFloat(map(accelPedalPercent, ACCEL_PEDAL_THRESHOLD, PEDAL_MAX, CURRENT_SP_MIN, CURRENT_SP_MAX));

    // Turn brakelight on/off
    brakeUpdate();

    // Check cruise set and update cruiseVelSetpoint accordingly
    cruiseVelCheckUpdate();
}

/**
 * @brief Accelerate Cruise State Decider. Determines transitions out of accelerate
 * cruise state (brake, neutral drive, one pedal, forward drive, record velocity,
 * coasting cruise).
 */
void AccelerateCruiseDecider()
{
    // If you're no longer in FORWARD_GEAR or you're braking, exit cruise
    if (brakePedalPercent == BRAKE_PRESSED || gear == PARK_GEAR || gear == REVERSE_GEAR)
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
void Task_SendTritium(void *p_arg)
{
    OS_ERR err;

    // Initialize current state to PARK_STATE
    state = FSM[PARK_STATE];
    prevState = FSM[PARK_STATE];
    UpdateDisplay_SetGear(PARK_GEAR);

    // Initialize Regen & Cruise disabled
    // NOTE: No regen on Daybreak
    UpdateDisplay_SetRegenState(DISP_DISABLED);
    UpdateDisplay_SetCruiseState(DISP_DISABLED);

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
        memcpy(&powerCmd.data[4], &busCurrentSetPoint, sizeof(float)); // CAN message for setpoint of bus current percent
        CANbus_Send(powerCmd, CAN_BLOCKING, MOTORCAN); 
        state.stateHandler();                          // do what the current state does
#ifndef SENDTRITIUM_EXPOSE_VARS
        readInputs(); // read inputs from the system
        UpdateDisplay_SetAccel(accelPedalPercent);
#endif
        prevState = state;
        state.stateDecider(); // decide what the next state is

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

static void assertSendTritiumError(SendTritium_error_code_t sterr)
{
    Error_SendTritium = (error_code_t) sterr;

    switch(sterr)
    {
        case SENDTRITIUM_ERR_NONE:
            break;
        case SENDTRITIUM_ERR_GEAR_FAULT:
            // Assert a nonrecoverable error that will kill the motor, turn off contactors, display a fault screen, & infinite loop
            throwTaskError(Error_SendTritium, NULL, OPT_LOCK_SCHED, OPT_NONRECOV);
    }

    Error_SendTritium = SENDTRITIUM_ERR_NONE;
}