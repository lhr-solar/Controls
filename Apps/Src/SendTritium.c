/**
 * @copyright Copyright (c) 2022 UT Longhorn Racing Solar
 * 
 * @file SendTritium.c
 * @brief Function implementations for the SendTritium application.
 * 
 * This contains functions relevant to updating the velocity and current setpoitns
 * of the Tritium motor controller. The implementation includes a normal current
 * controlled mode, a one-pedal driving mode, cruise control, and regenerative braking.
 * The logic is determined through a finite state machine implementation.
 * 
 * @author Nathaniel Delgado (NathanielDelgado)
 * @author Diya Rajon (diyarajon)
 * @author Ishan Deshpande (IshDeshpa)
 */

#include "Pedals.h"
#include "ReadCarCAN.h"
#include "Minions.h"
#include "SendTritium.h"
#include "ReadTritium.h"
#include "CANbus.h"


// Macros
#define MAX_VELOCITY 50.0f // m/s    if rmp: 20000.0f // rpm
#define NORMAL_BRAKE_THRESHOLD 5  // percent
#define ONEPEDAL_BRAKE_THRESHOLD 20  // percent
#define NEUTRAL_THRESHOLD 25    // percent
#define DEBOUNCE_PERIOD 2 // in 100 ms
#define VELOCITY_THRESHOLD 20.0f // m/s

#define MOTOR_MSG_PERIOD 100
#define FSM_PERIOD 20
#define MOTOR_MSG_COUNTER_THRESHOLD (MOTOR_MSG_PERIOD)/(FSM_PERIOD)

#ifdef __TEST_SENDTRITIUM
#define SCOPE
#else
#define SCOPE static
static int MotorCounter = 0;
#endif

// Inputs
SCOPE bool cruiseEnable = false;
SCOPE bool cruiseSet = false;
SCOPE bool regenToggle = false;
SCOPE uint8_t brakePedalPercent = 0;
SCOPE uint8_t accelPedalPercent = 0;
SCOPE bool reverseSwitch = false;
SCOPE bool forwardSwitch = false;

// Outputs
SCOPE float currentSetpoint = 0;
SCOPE float velocitySetpoint = 0;
SCOPE float cruiseVelSetpoint = 0;
static bool brakelightState = false;

#ifndef __TEST_SENDTRITIUM
// Debouncing counters
static uint8_t regenCounter = 0;
static uint8_t cruiseEnableCounter = 0;
static uint8_t cruiseSetCounter = 0;

// Button states
static bool regenButton = false;
static bool regenPrevious = false;

static bool cruiseEnableButton = false;
static bool cruiseEnablePrevious = false;

#else
float velocityObserved = 0;
#endif

// State Names
typedef enum{
    NORMAL_DRIVE,
    RECORD_VELOCITY,
    POWERED_CRUISE,
    COASTING_CRUISE,
    BRAKE_STATE,
    ONEPEDAL_DRIVE,
    ACCELERATE_CRUISE
} TritiumStateName_e;

// State Struct for FSM
typedef struct TritiumState{
    void (*stateHandler)(void);
    void (*stateDecider)(void);
} TritiumState_t;

// Current state
SCOPE TritiumStateName_e state = NORMAL_DRIVE;

// Handler & Decider Declarations
void NormalDriveHandler(void);
void NormalDriveDecider(void);
void RecordVelocityHandler(void);
void RecordVelocityDecider(void);
void PoweredCruiseHandler(void);
void PoweredCruiseDecider(void);
void CoastingCruiseHandler(void);
void CoastingCruiseDecider(void);
void BrakeHandler(void);
void BrakeDecider(void);
void OnePedalDriveHandler(void);
void OnePedalDriveDecider(void);
void AccelerateCruiseHandler(void);
void AccelerateCruiseDecider(void);

// FSM
const TritiumState_t FSM[9] = {
    {&NormalDriveHandler, &NormalDriveDecider},
    {&RecordVelocityHandler, &RecordVelocityDecider},
    {&PoweredCruiseHandler, &PoweredCruiseDecider},
    {&CoastingCruiseHandler, &CoastingCruiseDecider},
    {&BrakeHandler, &BrakeDecider},
    {&OnePedalDriveHandler, &OnePedalDriveDecider},
    {&AccelerateCruiseHandler, &AccelerateCruiseDecider}
};

/**
 * @brief   Reads inputs from the system
*/
void readInputs(){

    #ifndef __TEST_SENDTRITIUM
    Minion_Error_t err;

    brakePedalPercent = Pedals_Read(BRAKE);
    accelPedalPercent = Pedals_Read(ACCELERATOR);
    
    // UPDATE BUTTONS
    if(Minion_Read_Input(REGEN_SW, &err) && regenCounter < DEBOUNCE_PERIOD){regenCounter++;}
    else if(regenCounter > 0){regenCounter--;}

    if(Minion_Read_Input(CRUZ_EN, &err) && cruiseEnableCounter < DEBOUNCE_PERIOD){cruiseEnableCounter++;}
    else if(cruiseEnableCounter > 0){cruiseEnableCounter--;}

    if(Minion_Read_Input(CRUZ_EN, &err) && cruiseSetCounter < DEBOUNCE_PERIOD){cruiseSetCounter++;}
    else if(cruiseSetCounter > 0){cruiseSetCounter--;}
    
    forwardSwitch = Minion_Read_Input(FOR_SW, &err);
    reverseSwitch = Minion_Read_Input(REV_SW, &err);

    if(forwardSwitch){
        cruiseSet = Minion_Read_Input(CRUZ_ST, &err);
    }
    else{
        cruiseEnable = false;
        cruiseSet = false;
        regenToggle = false;
    }


    // DEBOUNCING
    if(regenCounter == DEBOUNCE_PERIOD){regenButton = true;}
    else if(regenCounter == 0){regenButton = false;}

    if(cruiseEnableCounter == DEBOUNCE_PERIOD){cruiseEnableButton = true;}
    else if(cruiseEnableCounter == 0){cruiseEnableButton = false;}

    if(cruiseSetCounter == DEBOUNCE_PERIOD){cruiseSet = true;}
    else if(cruiseSetCounter == 0){cruiseSet = false;}

    // TOGGLE
    if(ChargeEnable && regenButton != regenPrevious && regenPrevious){regenToggle = !regenToggle;}
    regenPrevious = regenButton;
    
    if(cruiseEnableButton != cruiseEnablePrevious && cruiseEnablePrevious){cruiseEnable = !cruiseEnable;}
    cruiseEnablePrevious = cruiseEnableButton;

    #endif
}

/**
 * @brief Converts integer percentage to float percentage
 * @param percent integer percentage from 0-100
*/
extern const float pedalToPercent[];
static float percentToFloat(uint8_t percent){
    if(percent > 100){
        return 1.0f;
    }
    return pedalToPercent[percent];
}

/**
 * @brief Follows the FSM to update the velocity of the car
*/
void Task_SendTritium(void *p_arg){
    OS_ERR err;
    Minion_Error_t minion_err;

#ifndef __TEST_SENDTRITIUM
    CANPayload_t velocityPayload;
            velocityPayload.bytes = 32;
            velocityPayload.data.d = velocitySetpoint;

    CANPayload_t currentPayload;
            currentPayload.bytes = 32;
            currentPayload.data.d = currentSetpoint;
#endif

    while(1){
        FSM[state].stateHandler();    // do what the current state does
        readInputs();   // read inputs from the system
        FSM[state].stateDecider();    // decide what the next state is

        // Sets brakelight
        Minion_Write_Output(BRAKELIGHT, brakelightState, &minion_err);

        // Drive
        #ifndef __TEST_SENDTRITIUM
         if(MOTOR_MSG_COUNTER_THRESHOLD%MotorCounter == 0){ // if smaller numbers, MOTOR_MSG_COUNTER_THRESHOLD-MotorCounter == 0
            MotorCounter = 0;

           // MotorController_Drive(velocitySetpoint, currentSetpoint);
            CANbus_Send(VELOCITY, velocityPayload, CAN_NON_BLOCKING);
            CANbus_Send(CURRENT_VEC, currentPayload, CAN_NON_BLOCKING);

        }else{
            MotorCounter++;
        }
        #endif

        // Dump Info for Testing
        #ifdef __TEST_SENDTRITIUM
        printf("-------------------\n\r");
        printf("State: %d\n\r", state);
        printf("cruiseEnable: %d\n\r", cruiseEnable);
        printf("cruiseSet: %d\n\r", cruiseSet);
        printf("regenToggle: %d\n\r", regenToggle);
        printf("brakePedalPercent: %d\n\r", brakePedalPercent);
        printf("accelPedalPercent: %d\n\r", accelPedalPercent);
        printf("reverseSwitch: %d\n\r", reverseSwitch);
        printf("forwardSwitch: %d\n\r", forwardSwitch);
        printf("currentSetpoint: %f\n\r", currentSetpoint);
        printf("velocitySetpoint: %f\n\r", velocitySetpoint);
        printf("velocityObserved: %f\n\r", velocityObserved);
        printf("-------------------\n\r");
        #endif


        // Delay of few milliseconds (100)
        OSTimeDlyHMSM(0, 0, 0, 20, OS_OPT_TIME_HMSM_STRICT, &err);
        if (err != OS_ERR_NONE){
            assertOSError(OS_UPDATE_VEL_LOC, err);
        }
    }
}

// State Handlers & Deciders

/**
 * Normal Drive State. Accelerator is mapped directly to current setpoint.
*/
void NormalDriveHandler(){
    if(reverseSwitch && !forwardSwitch){
        velocitySetpoint = -MAX_VELOCITY;
        currentSetpoint = percentToFloat(accelPedalPercent);
    }else if(!reverseSwitch && forwardSwitch){
        velocitySetpoint = MAX_VELOCITY;
        currentSetpoint = percentToFloat(accelPedalPercent);
    }else if(!reverseSwitch && !forwardSwitch){
        currentSetpoint = 0;
    }
}

void NormalDriveDecider(){
    if(cruiseSet && cruiseEnable && brakePedalPercent < NORMAL_BRAKE_THRESHOLD  && velocitySetpoint > VELOCITY_THRESHOLD){
        state = RECORD_VELOCITY;
    }else if(!cruiseEnable && regenToggle && ChargeEnable){
        state = ONEPEDAL_DRIVE;
    }else if(brakePedalPercent >= NORMAL_BRAKE_THRESHOLD){
        state = BRAKE_STATE;
    }
}

/**
 * Record Velocity State. While pressing the cruise set button, the car will record the observed velocity
 * into velocitySetpoint.
*/
void RecordVelocityHandler(){
    #ifndef __TEST_SENDTRITIUM
    velocitySetpoint = MotorVelocity_Get();
    #else
    velocitySetpoint = velocityObserved;
    #endif

}

void RecordVelocityDecider(){
    if(brakePedalPercent > NORMAL_BRAKE_THRESHOLD){
        state = BRAKE_STATE;
    }else if(cruiseEnable && !cruiseSet){
        state = POWERED_CRUISE;
    }else if(!cruiseEnable){
        state = NORMAL_DRIVE;
    }
}

/**
 * Powered Cruise State. Continue to travel at the recorded velocity as long as
 * Observed Velocity <= Velocity Setpoint
*/
void PoweredCruiseHandler(){
    currentSetpoint = 1.0f;
}

void PoweredCruiseDecider(){
    #ifndef __TEST_SENDTRITIUM
    cruiseVelSetpoint = velocitySetpoint;
    if(MotorVelocity_Get() > velocitySetpoint){
        state = COASTING_CRUISE;
    }
    #else
    if(velocityObserved > velocitySetpoint){
        state = COASTING_CRUISE;
    }
    #endif
    else if(!cruiseEnable){
        state = NORMAL_DRIVE;
    }else if(cruiseEnable && cruiseSet && velocitySetpoint > VELOCITY_THRESHOLD){
        state = RECORD_VELOCITY;
    }else if(brakePedalPercent >= NORMAL_BRAKE_THRESHOLD){
        state = BRAKE_STATE;
    }else if(regenToggle){
        cruiseEnable = false;
        state = NORMAL_DRIVE;
    }
}

/**
 * Coasting Cruise State. We do not want to utilize motor braking in cruise control mode.
 * Coast the motor (do not motor brake) if we want to slow down; instead, stop putting power
 * into the motor and let the car slow down naturally.
*/
void CoastingCruiseHandler(){
    currentSetpoint = 0;
}

void CoastingCruiseDecider(){
    if(MotorVelocity_Get() <= velocitySetpoint){
        state = POWERED_CRUISE;
    }else if(!cruiseEnable){
        state = NORMAL_DRIVE;
    }else if(brakePedalPercent >= NORMAL_BRAKE_THRESHOLD){
        state = BRAKE_STATE;
    }else if(regenToggle){
        cruiseEnable = false;
        state = NORMAL_DRIVE;
    }
}

/**
 * Brake State. Brake the car if the brake pedal is pressed.
*/
void BrakeHandler(){
    currentSetpoint = 0;
    velocitySetpoint = 0;
    cruiseEnable = 0;
    cruiseSet = 0;
    brakelightState = true;
}

void BrakeDecider(){
    if(brakePedalPercent < NORMAL_BRAKE_THRESHOLD){
        state = NORMAL_DRIVE;
        brakelightState = false;
    }
}

/**
 * One Pedal Drive State. When in one pedal drive, if the accelerator percentage is lower
 * than ONEPEDAL_BRAKE_THRESHOLD, the car will utilize motor braking to slow down. If accelerator
 * percentage is in the neutral zone, the car will coast. If accelerator percentage is above
 * the NEUTRAL_THRESHOLD, the car will accelerate as normal.
*/
void OnePedalDriveHandler(){
    if(accelPedalPercent < ONEPEDAL_BRAKE_THRESHOLD){
        // Motor brake
        currentSetpoint = percentToFloat(((ONEPEDAL_BRAKE_THRESHOLD - accelPedalPercent)*100)/ONEPEDAL_BRAKE_THRESHOLD);
        velocitySetpoint = 0;
        brakelightState = true;
    }else if(accelPedalPercent < NEUTRAL_THRESHOLD  && accelPedalPercent >= ONEPEDAL_BRAKE_THRESHOLD){
        // Coast
        currentSetpoint = 0;
        brakelightState = false;
    }else if(accelPedalPercent >= NEUTRAL_THRESHOLD){
        // Accelerate
        currentSetpoint = percentToFloat(((accelPedalPercent - NEUTRAL_THRESHOLD)*100)/(100-NEUTRAL_THRESHOLD));
        brakelightState = false;
    }
}

void OnePedalDriveDecider(){
    if(!regenToggle || !ChargeEnable || cruiseEnable){
        regenToggle = 0;
        state = NORMAL_DRIVE;
    }else if(brakePedalPercent >= NORMAL_BRAKE_THRESHOLD){
        state = BRAKE_STATE;
    }
}


void AccelerateCruiseHandler(){
    if(reverseSwitch && !forwardSwitch){
        velocitySetpoint = -MAX_VELOCITY;
        currentSetpoint = percentToFloat(accelPedalPercent);
    }else if(!reverseSwitch && forwardSwitch){
        velocitySetpoint = MAX_VELOCITY;
        currentSetpoint = percentToFloat(accelPedalPercent);
    }else if(!reverseSwitch && !forwardSwitch){
        currentSetpoint = 0;
    }
}

void AccelerateCruiseDecider(){
    if(accelPedalPercent == 0){
        state = COASTING_CRUISE;
    }else if(brakePedalPercent >= NORMAL_BRAKE_THRESHOLD){
        state = BRAKE_STATE;
    }
}
