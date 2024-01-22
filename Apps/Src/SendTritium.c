/**
 * @file SendTritium.c
 * 
 * SendTritium utilizes an FSM to coordinate what messages should be sent to the motor 
 * controller depending on a certain set of input variables. This FSM depends 
 * on data from across the system, such as the [Pedals](../Drivers/Pedals.html), 
 * the [Switches](../Drivers/Minions.html), and the CAN messages from 
 * BPS (indicating whether charging is enabled).
 * 
 * The FSM has three major modes of operation, which are:
 * - "Normal" (Current Controlled) mode. This includes the Forward, Neutral, and Reverse states.
 * - One Pedal Drive (Velocity Controlled) mode. This is the state where the driver is only using the accelerator pedal to control the car.
 * - Cruise Control (Velocity Controlled) mode. This is the state where the driver is using the cruise control buttons to control the car. This includes the Record Velocity, Powered Cruise, Coasting Cruise, and Accelerate Cruise states.
 * The FSM also includes the brake state, which disables One Pedal Drive or Cruise Control and puts the car in neutral to allow the physical brakes to be used.
 * 
 * **Normal** mode allows the driver to control the car with the accelerator mapped directly to the current setpoint, and the brake pedal tied to the physical brake. 
 * This is similar to operation of a typical vehicle. This includes three states for the three gears: Forward, Neutral, and Reverse.
 *
 * **One Pedal** Drive mode allows the driver to control the car with the accelerator pedal only. A certain percentage of the pedal is mapped to regenerative braking,
 * a certain percentage is mapped to coasting, and the rest is mapped to acceleration. This means that if the driver lifts their foot completely off the pedal, the car
 * will come to a halt.
 * 
 * **Cruise Control** mode allows the driver to control the car with the cruise control buttons. The driver can set the cruise control speed by pressing the *Cruise Set* button,
 * and the car will maintain that speed until the driver presses the brake pedal. The driver can also accelerate or decelerate the car by pressing the accelerator pedal (this will enter the 
 * Accelerate Cruise state). 
 * - A concern with cruise control is that the vehicle operates in the motor controller's Velocity Controlled mode, which uses motor braking. Since the car has a one pedal drive, the use
 * of motor braking in conjunction with physical braking can cause the car to spin out due to a force differential. To mitigate this, the Powered Cruise/Coasting Cruise distinction was made,
 * where the car will coast if the observed velocity is greater than the setpoint (Coasting Cruise), and will use velocity control if the observed velocity is less than the setpoint (Powered Cruise).
 * 
 * # Implementation Details
 * If the macro SENDTRITIUM_EXPOSE_VARS is defined prior to including `SendTritium.h`, 
 * relevant setters will be exposed as externs for unit testing
 * and hardware inputs won't be read and motor commands won't be sent over MotorCAN.
 * If the macro SENDTRITIUM_PRINT_MES is also defined prior to including `SendTritium.h`,
 * debug info will be printed via UART.
 * 
 * # FSM Diagram
 * @image xml SendTritiumFSM.png
 * 
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
/**
 * @brief Threshold for the motor message counter
*/
#define MOTOR_MSG_COUNTER_THRESHOLD (MOTOR_MSG_PERIOD)/(FSM_PERIOD)

/**
 * @brief Maximum velocity of the car in rpm (unobtainable value)
*/
#define MAX_VELOCITY 20000.0f

// Velocity Limits
/**
 * @brief Minimum velocity of the car in rpm to enable cruise control
*/
#define MIN_CRUISE_VELOCITY MpsToRpm(20.0f)

/**
 * @brief Maximum velocity of the car in rpm to switch gears
*/
#define MAX_GEARSWITCH_VELOCITY MpsToRpm(8.0f)

// Deadbands & Thresholds
/**
 * @brief Minimum brake pedal percentage to engage brake state
*/
#define BRAKE_PEDAL_DEADBAND 15

/**
 * @brief Minimum accelerator pedal percentage to engage acceleration in 
 * normal drive mode
*/
#define ACCEL_PEDAL_DEADBAND 10

/**
 * @brief One pedal drive mode upper limit for regenerative braking
*/
#define ONEPEDAL_BRAKE_THRESHOLD 25

/**
 * @brief One pedal drive mode upper limit for neutral
*/
#define ONEPEDAL_NEUTRAL_THRESHOLD 35

/**
 * @brief Minimum pedal percentage
*/
#define PEDAL_MIN 0

/**
 * @brief Maximum pedal percentage
*/
#define PEDAL_MAX 100

/**
 * @brief Minimum current setpoint percentage
*/
#define CURRENT_SP_MIN 0

/**
 * @brief Maximum current setpoint percentage
*/
#define CURRENT_SP_MAX 100

/**
 * @brief Number of times gear fault can occur before it is considered a fault
*/
#define GEAR_FAULT_THRESHOLD 3

// Inputs
static bool cruise_enable = false;
static bool cruise_set = false;
static bool one_pedal_enable = false;
static bool regen_enable = false;

static uint8_t brake_pedal_percent = 0;
static uint8_t accel_pedal_percent = 0;

static Gear gear = kNeutralGear;

// Outputs
float current_setpoint = 0;
float velocity_setpoint = 0;
float cruise_vel_setpoint = 0;

// Current observed velocity
static float velocity_observed = 0;

#ifndef SENDTRITIUM_EXPOSE_VARS
// Counter for sending setpoints to motor
static uint8_t motor_msg_counter = 0;

// Debouncing counters
static uint8_t one_pedal_counter = 0;
static uint8_t cruise_enable_counter = 0;
static uint8_t cruise_set_counter = 0;

// Button states
static bool one_pedal_button = false;
static bool one_pedal_previous = false;

static bool cruise_enable_button = false;
static bool cruise_enable_previous = false;
#endif

// FSM
static TritiumState prev_state;  // Previous state
static TritiumState state;       // Current state

bool GetCruiseEnable(void) { return cruise_enable; }
bool GetCruiseSet(void) { return cruise_set; }
bool GetOnePedalEnable(void) { return one_pedal_enable; }
bool GetRegenEnable(void) { return regen_enable; }
uint8_t GetBrakePedalPercent(void) { return brake_pedal_percent; }
uint8_t GetAccelPedalPercent(void) { return accel_pedal_percent; }
Gear GetGear(void) { return gear; }
TritiumState GetState(void) { return state; }
float GetVelocityObserved(void) { return velocity_observed; }
float GetCruiseVelSetpoint(void) { return cruise_vel_setpoint; }
float GetCurrentSetpoint(void) { return current_setpoint; }
float GetVelocitySetpoint(void) { return velocity_setpoint; }

#ifdef SENDTRITIUM_EXPOSE_VARS
void SetCruiseEnable(bool value) { cruise_enable = value; }
void SetCruiseSet(bool value) { cruise_set = value; }
void SetOnePedalEnable(bool value) { one_pedal_enable = value; }
void SetRegenEnable(bool value) { regen_enable = value; }
void SetBrakePedalPercent(uint8_t value) { brake_pedal_percent = value; }
void SetAccelPedalPercent(uint8_t value) { accel_pedal_percent = value; }
void SetGear(Gear value) { gear = value; }
void SetState(TritiumState value) { state = value; }
void SetVelocityObserved(float value) { velocity_observed = value; }
void SetCruiseVelSetpoint(float value) { cruise_vel_setpoint = value; }
void SetCurrentSetpoint(float value) { current_setpoint = value; }
void SetVelocitySetpoint(float value) { velocity_setpoint = value; }
#endif

// Handler & Decider Declarations
static void forwardDriveHandler(void);
static void forwardDriveDecider(void);
static void neutralDriveHandler(void);
static void neutralDriveDecider(void);
static void reverseDriveHandler(void);
static void reverseDriveDecider(void);
static void recordVelocityHandler(void);
static void recordVelocityDecider(void);
static void poweredCruiseHandler(void);
static void poweredCruiseDecider(void);
static void coastingCruiseHandler(void);
static void coastingCruiseDecider(void);
static void brakeHandler(void);
static void brakeDecider(void);
static void onePedalDriveHandler(void);
static void onePedalDriveDecider(void);
static void accelerateCruiseHandler(void);
static void accelerateCruiseDecider(void);

// FSM
static const TritiumState kFsm[9] = {
    {kForwardDrive, &forwardDriveHandler, &forwardDriveDecider},
    {kNeutralDrive, &neutralDriveHandler, &neutralDriveDecider},
    {kReverseDrive, &reverseDriveHandler, &reverseDriveDecider},
    {kRecordVelocity, &recordVelocityHandler, &recordVelocityDecider},
    {kPoweredCruise, &poweredCruiseHandler, &poweredCruiseDecider},
    {kCoastingCruise, &coastingCruiseHandler, &coastingCruiseDecider},
    {kBrakeState, &brakeHandler, &brakeDecider},
    {kOnePedal, &onePedalDriveHandler, &onePedalDriveDecider},
    {kAccelerateCruise, &accelerateCruiseHandler, &accelerateCruiseDecider}};

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
        case kNeutralDrive:
            strcpy(name_str, "NEUTRAL_DRIVE");
            break;
        case kReverseDrive:
            strcpy(name_str, "REVERSE_DRIVE");
            break;
        case kRecordVelocity:
            strcpy(name_str, "RECORD_VELOCITY");
            break;
        case kPoweredCruise:
            strcpy(name_str, "POWERED_CRUISE");
            break;
        case kCoastingCruise:
            strcpy(name_str, "COASTING_CRUISE");
            break;
        case kBrakeState:
            strcpy(name_str, "BRAKE_STATE");
            break;
        case kOnePedal:
            strcpy(name_str, "ONEPEDAL");
            break;
        case kAccelerateCruise:
            strcpy(name_str, "ACCELERATE_CRUISE");
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
    printf("cruiseEnable: %d\n\r", cruise_enable);
    printf("cruiseSet: %d\n\r", cruise_set);
    printf("onePedalEnable: %d\n\r", one_pedal_enable);
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

    // Update regen enable
    regen_enable = ChargeEnableGet();

    // Update buttons
    if (MinionsRead(kRegenSw) && one_pedal_counter < DEBOUNCE_PERIOD) {
        one_pedal_counter++;
    } else if (one_pedal_counter > 0) {
        one_pedal_counter--;
    }

    if (MinionsRead(kCruzEn) && cruise_enable_counter < DEBOUNCE_PERIOD) {
        cruise_enable_counter++;
    } else if (cruise_enable_counter > 0) {
        cruise_enable_counter--;
    }

    if (MinionsRead(kCruzSt) && cruise_set_counter < DEBOUNCE_PERIOD) {
        cruise_set_counter++;
    } else if (cruise_set_counter > 0) {
        cruise_set_counter--;
    }

    // Update gears
    bool forward_switch = MinionsRead(kForSw);
    bool reverse_switch = MinionsRead(kRevSw);
    bool forward_gear = (forward_switch && !reverse_switch);
    bool reverse_gear = (!forward_switch && reverse_switch);
    bool neutral_gear = (!forward_switch && !reverse_switch);

    uint8_t gear_fault =
        (uint8_t)forward_gear + (uint8_t)reverse_gear + (uint8_t)neutral_gear;
    static uint8_t gear_fault_cnt = 0;

    if (gear_fault != 1) {
        // Fault behavior
        if (gear_fault_cnt > GEAR_FAULT_THRESHOLD) {
            state = kFsm[kNeutralDrive];
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
        gear = kNeutralGear;
    }

    // Debouncing
    if (one_pedal_counter == DEBOUNCE_PERIOD) {
        one_pedal_button = true;
    } else if (one_pedal_counter == 0) {
        one_pedal_button = false;
    }

    if (cruise_enable_counter == DEBOUNCE_PERIOD) {
        cruise_enable_button = true;
    } else if (cruise_enable_counter == 0) {
        cruise_enable_button = false;
    }

    if (cruise_set_counter == DEBOUNCE_PERIOD) {
        cruise_set = true;
    } else if (cruise_set_counter == 0) {
        cruise_set = false;
    }

    // Toggle
    if (one_pedal_button != one_pedal_previous && one_pedal_previous) {
        one_pedal_enable = !one_pedal_enable;
    }
    if (!regen_enable) {
        one_pedal_enable = false;
    }
    one_pedal_previous = one_pedal_button;

    if (cruise_enable_button != cruise_enable_previous &&
        cruise_enable_previous) {
        cruise_enable = !cruise_enable;
    }
    cruise_enable_previous = cruise_enable_button;

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
        UpdateDisplaySetCruiseState(DISP_DISABLED);
        UpdateDisplaySetRegenState(DISP_DISABLED);
        UpdateDisplaySetGear(DISP_FORWARD);
    }

    velocity_setpoint = MAX_VELOCITY;
    current_setpoint =
        percentToFloat(map(accel_pedal_percent, ACCEL_PEDAL_DEADBAND,
                           PEDAL_MAX, CURRENT_SP_MIN, CURRENT_SP_MAX));
}

/**
 * @brief Forward Drive State Decider. Determines transitions out of
 * forward drive state (brake, record velocity, one pedal, neutral drive).
 */
static void forwardDriveDecider() {
    if (brake_pedal_percent >= BRAKE_PEDAL_DEADBAND) {
        state = kFsm[kBrakeState];
    } else if (cruise_set && cruise_enable &&
               velocity_observed >= MIN_CRUISE_VELOCITY) {
        state = kFsm[kRecordVelocity];
    } else if (one_pedal_enable) {
        state = kFsm[kOnePedal];
    } else if (gear == kNeutralGear || gear == kReverseGear) {
        state = kFsm[kNeutralDrive];
    }
}

/**
 * @brief Neutral Drive State Handler. No current is sent to the motor.
 */
static void neutralDriveHandler() {
    if (prev_state.name != state.name) {
        UpdateDisplaySetCruiseState(DISP_DISABLED);
        UpdateDisplaySetRegenState(DISP_DISABLED);
        UpdateDisplaySetGear(DISP_NEUTRAL);
    }
    velocity_setpoint = MAX_VELOCITY;
    current_setpoint = 0.0F;

    cruise_enable = false;
    one_pedal_enable = false;
}

/**
 * @brief Neutral Drive State Decider. Determines transitions out of
 * neutral drive state (brake, forward drive, reverse drive).
 */
static void neutralDriveDecider() {
    if (brake_pedal_percent >= BRAKE_PEDAL_DEADBAND) {
        state = kFsm[kBrakeState];
    } else if (gear == kForwardGear &&
               velocity_observed >= -MAX_GEARSWITCH_VELOCITY) {
        state = kFsm[kForwardDrive];
    } else if (gear == kReverseGear &&
               velocity_observed <= MAX_GEARSWITCH_VELOCITY) {
        state = kFsm[kReverseDrive];
    }
}

/**
 * @brief Reverse Drive State Handler. Accelerator is mapped directly to
 * current setpoint (at negative velocity).
 */
static void reverseDriveHandler() {
    if (prev_state.name != state.name) {
        UpdateDisplaySetCruiseState(DISP_DISABLED);
        UpdateDisplaySetRegenState(DISP_DISABLED);
        UpdateDisplaySetGear(DISP_REVERSE);
    }
    velocity_setpoint = -MAX_VELOCITY;
    current_setpoint =
        percentToFloat(map(accel_pedal_percent, ACCEL_PEDAL_DEADBAND,
                           PEDAL_MAX, CURRENT_SP_MIN, CURRENT_SP_MAX));
    cruise_enable = false;
    one_pedal_enable = false;
}

/**
 * @brief Reverse Drive State Decider. Determines transitions out of
 * reverse drive state (brake, neutral drive).
 */
static void reverseDriveDecider() {
    if (brake_pedal_percent >= BRAKE_PEDAL_DEADBAND) {
        state = kFsm[kBrakeState];
    } else if (gear == kNeutralGear || gear == kForwardGear) {
        state = kFsm[kNeutralDrive];
    }
}

/**
 * @brief Record kVelocity State. While pressing the cruise set button,
 * the car will record the observed velocity into velocitySetpoint.
 */
static void recordVelocityHandler() {
    if (prev_state.name != state.name) {
        UpdateDisplaySetCruiseState(DISP_ACTIVE);
        UpdateDisplaySetRegenState(DISP_DISABLED);
        UpdateDisplaySetGear(DISP_FORWARD);
    }
    // put car in neutral while recording velocity (while button is held)
    velocity_setpoint = MAX_VELOCITY;
    current_setpoint = 0;
    cruise_vel_setpoint = velocity_observed;
}

/**
 * @brief Record Velocity State Decider. Determines transitions out of record velocity 
 * state (brake, neutral drive, one pedal, forward drive, powered cruise).
*/
static void recordVelocityDecider() {
    if (brake_pedal_percent >= BRAKE_PEDAL_DEADBAND) {
        state = kFsm[kBrakeState];
    } else if (gear == kNeutralGear || gear == kReverseGear) {
        state = kFsm[kNeutralDrive];
    } else if (one_pedal_enable) {
        cruise_enable = false;
        state = kFsm[kOnePedal];
    } else if (!cruise_enable) {
        state = kFsm[kForwardDrive];
    } else if (cruise_enable && !cruise_set) {
        state = kFsm[kPoweredCruise];
    }
}

/**
 * @brief Powered Cruise State. Continue to travel at the recorded velocity as
 * long as Observed kVelocity <= kVelocity Setpoint
 */
static void poweredCruiseHandler() {
    velocity_setpoint = cruise_vel_setpoint;
    current_setpoint = 1.0F;
}

/**
 * @brief Powered Cruise State Decider. Determines transitions out of powered
 * cruise state (brake, neutral drive, one pedal, forward drive, record
 * velocity, accelerate cruise, coasting cruise).
 */
static void poweredCruiseDecider() {
    if (brake_pedal_percent >= BRAKE_PEDAL_DEADBAND) {
        state = kFsm[kBrakeState];
    } else if (gear == kNeutralGear || gear == kReverseGear) {
        state = kFsm[kNeutralDrive];
    } else if (one_pedal_enable) {
        cruise_enable = false;
        state = kFsm[kOnePedal];
    } else if (!cruise_enable) {
        state = kFsm[kForwardDrive];
    } else if (cruise_set && velocity_observed >= MIN_CRUISE_VELOCITY) {
        state = kFsm[kRecordVelocity];
    } else if (accel_pedal_percent >= ACCEL_PEDAL_DEADBAND) {
        state = kFsm[kAccelerateCruise];
    } else if (velocity_observed > cruise_vel_setpoint) {
        state = kFsm[kCoastingCruise];
    }
}

/**
 * @brief Coasting Cruise State. We do not want to utilize motor braking
 * in cruise control mode due to safety issues. Coast the motor (go into
 * neutral) if we want to slow down.
 */
static void coastingCruiseHandler() {
    velocity_setpoint = cruise_vel_setpoint;
    current_setpoint = 0;
}

/**
 * @brief Coasting Cruise State Decider. Determines transitions out of coasting
 * cruise state (brake, neutral drive, one pedal, forward drive, record
 * velocity, accelerate cruise, powered cruise).
 */
static void coastingCruiseDecider() {
    if (brake_pedal_percent >= BRAKE_PEDAL_DEADBAND) {
        state = kFsm[kBrakeState];
    } else if (gear == kNeutralGear || gear == kReverseGear) {
        state = kFsm[kNeutralDrive];
    } else if (one_pedal_enable) {
        cruise_enable = false;
        state = kFsm[kOnePedal];
    } else if (!cruise_enable) {
        state = kFsm[kForwardDrive];
    } else if (cruise_set && velocity_observed >= MIN_CRUISE_VELOCITY) {
        state = kFsm[kRecordVelocity];
    } else if (accel_pedal_percent >= ACCEL_PEDAL_DEADBAND) {
        state = kFsm[kAccelerateCruise];
    } else if (velocity_observed <= cruise_vel_setpoint) {
        state = kFsm[kPoweredCruise];
    }
}

/**
 * @brief Accelerate Cruise State. In the event that the driver needs to
 * accelerate in cruise mode, we will accelerate to the pedal percentage. Upon
 * release of the accelerator pedal, we will return to cruise mode at the
 * previously recorded velocity.
 */
static void accelerateCruiseHandler() {
    velocity_setpoint = MAX_VELOCITY;
    current_setpoint =
        percentToFloat(map(accel_pedal_percent, ACCEL_PEDAL_DEADBAND,
                           PEDAL_MAX, CURRENT_SP_MIN, CURRENT_SP_MAX));
}

/**
 * @brief Accelerate Cruise State Decider. Determines transitions out of
 * accelerate cruise state (brake, neutral drive, one pedal, forward drive,
 * record velocity, coasting cruise).
 */
static void accelerateCruiseDecider() {
    if (brake_pedal_percent >= BRAKE_PEDAL_DEADBAND) {
        state = kFsm[kBrakeState];
    } else if (gear == kNeutralGear || gear == kReverseGear) {
        state = kFsm[kNeutralDrive];
    } else if (one_pedal_enable) {
        cruise_enable = false;
        state = kFsm[kOnePedal];
    } else if (!cruise_enable) {
        state = kFsm[kForwardDrive];
    } else if (cruise_set && velocity_observed >= MIN_CRUISE_VELOCITY) {
        state = kFsm[kRecordVelocity];
    } else if (accel_pedal_percent < ACCEL_PEDAL_DEADBAND) {
        state = kFsm[kCoastingCruise];
    }
}

/**
 * @brief One Pedal Drive State. When in one pedal drive, if the accelerator
 * percentage is lower than ONEPEDAL_BRAKE_THRESHOLD, the car will utilize motor
 * braking to slow down. If accelerator percentage is in the neutral zone, the
 * car will coast. If accelerator percentage is above the NEUTRAL_THRESHOLD, the
 * car will accelerate as normal.
 */
static void onePedalDriveHandler() {
    if (prev_state.name != state.name) {
        UpdateDisplaySetCruiseState(DISP_DISABLED);
        UpdateDisplaySetGear(DISP_FORWARD);
    }
    if (accel_pedal_percent <= ONEPEDAL_BRAKE_THRESHOLD) {
        // Regen brake: Map 0 -> brake to 100 -> 0
        velocity_setpoint = 0;
        current_setpoint = percentToFloat(map(accel_pedal_percent, PEDAL_MIN,
                                              ONEPEDAL_BRAKE_THRESHOLD,
                                              CURRENT_SP_MAX, CURRENT_SP_MIN));
        MinionsWrite(kBrakeLight, true);
        UpdateDisplaySetRegenState(DISP_ACTIVE);
    } else if (ONEPEDAL_BRAKE_THRESHOLD < accel_pedal_percent &&
               accel_pedal_percent <= ONEPEDAL_NEUTRAL_THRESHOLD) {
        // Neutral: coast
        velocity_setpoint = MAX_VELOCITY;
        current_setpoint = 0;
        MinionsWrite(kBrakeLight, false);
        UpdateDisplaySetRegenState(DISP_ENABLED);
    } else if (ONEPEDAL_NEUTRAL_THRESHOLD < accel_pedal_percent) {
        // Accelerate: Map neutral -> 100 to 0 -> 100
        velocity_setpoint = MAX_VELOCITY;
        current_setpoint =
            percentToFloat(map(accel_pedal_percent, ONEPEDAL_NEUTRAL_THRESHOLD,
                               PEDAL_MAX, CURRENT_SP_MIN, CURRENT_SP_MAX));
        MinionsWrite(kBrakeLight, false);
        UpdateDisplaySetRegenState(DISP_ENABLED);
    }
}

/**
 * @brief One Pedal Drive State Decider. Determines transitions out of one pedal
 * drive state (brake, record velocity, neutral drive).
 */
static void onePedalDriveDecider() {
    if (brake_pedal_percent >= BRAKE_PEDAL_DEADBAND) {
        state = kFsm[kBrakeState];
    } else if (cruise_set && cruise_enable &&
               velocity_observed >= MIN_CRUISE_VELOCITY) {
        state = kFsm[kRecordVelocity];
        MinionsWrite(kBrakeLight, false);
    } else if (gear == kNeutralGear || gear == kReverseGear) {
        state = kFsm[kNeutralDrive];
        MinionsWrite(kBrakeLight, false);
    }
}

/**
 * @brief Brake State. When brake pedal is pressed, physical brakes will be
 * active. Put motor in neutral to prevent motor braking while physical brakes
 * are engaged. Additionally, disable all cruise control and one pedal
 * functionality.
 */
static void brakeHandler() {
    if (prev_state.name != state.name) {
        UpdateDisplaySetCruiseState(DISP_DISABLED);
        UpdateDisplaySetRegenState(DISP_DISABLED);
        UpdateDisplaySetGear(DISP_FORWARD);
    }
    velocity_setpoint = MAX_VELOCITY;
    current_setpoint = 0;
    cruise_enable = false;
    one_pedal_enable = false;
    MinionsWrite(kBrakeLight, true);
}

/**
 * @brief Brake State Decider. Determines transitions out of brake state (forward drive, 
 * neutral drive).
*/
static void brakeDecider() {
    if (brake_pedal_percent < BRAKE_PEDAL_DEADBAND) {
        if (gear == kForwardGear) {
            state = kFsm[kForwardDrive];
        } else if (gear == kNeutralGear || gear == kReverseGear) {
            state = kFsm[kNeutralDrive];
        }
        MinionsWrite(kBrakeLight, false);
    }
}

// Task (main loop)

/**
 * @brief Follows the FSM to update the velocity of the car
 */
void TaskSendTritium(void* p_arg) {
    OS_ERR err = 0;

    // Initialize current state to FORWARD_DRIVE
    state = kFsm[kNeutralDrive];
    prev_state = kFsm[kNeutralDrive];

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

        // Delay of FSM_PERIOD ms
        OSTimeDlyHMSM(0, 0, 0, FSM_PERIOD, OS_OPT_TIME_HMSM_STRICT, &err);
        if (err != OS_ERR_NONE){
            assertOSError(err);
        }
    }
}
