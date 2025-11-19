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

 #define NEXT_STATES_LENGTH 128

 #define BRAKE_THRESH 42
 #define BRAKE_THRESH_HYST 30
 #define REGEN_STEP 0.02f
 #define CAN_MSG_BUFFER_SIZE 1

 //CURRENT FSM STATE
 TritiumState_t currentState;


 typedef struct FSMCANDATA{ //May want to add more later..
    CANDATA_t CANMessage;
    uint64_t timestamp;
 }FSMCANDATA_t;

static float brakePedalPercent = 0.0f;
static float accelPedalPercent = 0.0f;
static float busCurrentSetPoint = 1.0f; //This gets manipulated if the battery not ok

static gear_t gear = DASH_NEU;

// Gear fault counter
static uint8_t gearFaultCnt = 0;

static bool isBrakeOn = false; // Used for updating display & brakelight

static uint8_t carStatus = 0; // Bitfield for car status

bool accelerator_reset  = false;

// Outputs
// static float currentSetpoint = 0.0f;
// static float velocitySetpoint = 0.0f;
int thresholdBrake = BRAKE_THRESH;

//CAN messages
static FSMCANDATA_t CAR_MSGS[NUM_CARCAN_FILTERS];

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

// NOTE: Instead of a "velocityObserved" variable, we can just use Motor_Velocity_Get() from
// ReadTritium when doing cruise logic

// Getter functions for local variables in SendTritium.c
GETTER(uint8_t, brakePedalPercent)
GETTER(uint8_t, accelPedalPercent)
GETTER(gear_t, gear)
GETTER(float, currentSetpoint)
GETTER(float, velocitySetpoint)
GETTER(bool, isBrakeOn)




//Create the FSM data strcuture, make all the possible states, to do this prolly use 
// an enum for every state and have a decision handler for each, logic will 
// be embedded within this, alos we would have flags represented by bits for whatever we care about

typedef enum FSMStates {
    STATE_INIT = 0,
    FORWARD_DRIVE = 1,
    NEUTRAL = 2,
    REVERSE_DRIVE = 3,
    REGEN = 4,
    CRUISE_CONTROL = 5,
    DISABLED = 6,
    CAR_NOT_READY = 7,
    NUM_STATES = 8
    // Add more states as needed
}FSMStates;

//BITFIELD INPUT ENUM
typedef enum BitfieldInputs {
    NEUTRAL_BIT             = 0x00, // If we are trying to go neutral
    FORWARD_BIT             = 0x01, // If we are trying to go forward
    REVERSE_BIT             = 0x02, // If we are trying to go reverse
    NOT_READY_BITS          = 0x03, // For when the car is starting up
    CRUISE_CONTROL_BUTTON_BIT = 0x04, // If the cruise control button is pressed
    REGEN_BUTTON_BIT        = 0x08, // If the regen button is pressed
    READY_TO_REGEN_BIT      = 0x10, // If we are going slow enough to regen
    REGEN_ENABLED_BIT       = 0x20, // If regen is enabled
    BRAKING_BIT             = 0x40, // If the brake is pressed
    //FAULTED_BIT           = 0x80  // If the car is faulted
} BitfieldInputs_t;

OS_FLAG_GRP CarStatus_Flags; // Bitfield for car status

typedef struct TritiumState{
    FSMStates stateName;
    void (*stateHandler)(void);
    int NextStates[NEXT_STATES_LENGTH]; //Default is neutral
} TritiumState_t;

TritiumState_t FSM[NUM_STATES] = {
    {STATE_INIT, &handleFSMInitState, {0}},
    {CAR_NOT_READY, &handleFSMNotReadyState, {0}},
    {FORWARD_DRIVE, &handleFSMForwardDriveState, {0}},
    {NEUTRAL, &handleFSMNeutralState, {0}},
    {REVERSE_DRIVE, &handleFSMReverseDriveState, {0}},
    {REGEN, &handleFSMRegenState, {0}},
    {CRUISE_CONTROL, &handleFSMCruiseControlState, {0}},
    {DISABLED, &handleFSMDisabledState, {0}},
};

//Todo: Make an RTOS Task for this one with watchdogs for BPS, Pedals, and gear, make a display function,  have another task searching for BPS and updating state, and update regen not ready, and then finally display all the faults..
//Questions: Do i need to make an rtos task for all this? Am i just replacing read/send tritium or also read car can (ignition + IO stuff tracking + faults anyways)? Can i throw a flag into not ready for BPS stuff? 
void runFSM(){
    OS_ERR err;

    // Check that motor is ready to run
    err = checkForAllFaults();

    // if you return OS_ERR_PEND_WOULD_BLOCK, one of the bits are not sent, and would've blocked
    // If the error is ERR_NONE, assertOSError returns without asserting an error
    if (err != OS_ERR_PEND_WOULD_BLOCK) { //What this mean?
        sendMotorPowerCommand(0.0f); //Battery tweaking = stop current
        assertOSError(err);
    }
    
    memset(&motorSafeCmd.data, 0, sizeof(motorSafeCmd.data)); //not safe to run
    sendMotorPowerCommand(busCurrentSetPoint); //Current should still be allowed in the case of recovery as well

    if(err == OS_ERR_NONE){
        motorSafeCmd.data[0] |= 0x01; // Set motor safe to run to true
        carStatus = getControlsBitfield(); //Get the current status of the car as a bitfield
        currentState = FSM[currentState.NextStates[carStatus]]; //Get the next state based on the current state and car status
        currentState.stateHandler(); //Run the handler for the current state
    }else{
        //If we are here, we are not ready to run, so set the state to not ready
        currentState = FSM[CAR_NOT_READY]; //Recoverable?
        currentState.stateHandler();
    }

    SendCarCAN_Put(motorSafeCmd); // Send the motor safe command, don't know why we do this
}

void init(){
    currentState = FSM[STATE_INIT]; //Start in init state
    currentState.stateHandler(); //Run init handler
}

void initFSM(){
    //The states in the for loop may be overwritten later, they are just a base

    for(int i = 0 ; i < NUM_STATES; i++){
        
        for(int j = 0; j < NEXT_STATES_LENGTH - 1; j++){
            if(i == CAR_NOT_READY){
                FSM[i].NextStates[j] = CAR_NOT_READY; //If we are not ready, stay in this state
                continue;
            }

            if(i == DISABLED){
                FSM[i].NextStates[j] = DISABLED; //If we are disabled, stay in this state
                continue;
            }

            if(j >= BRAKING_BIT){
                //Braking, -> check regen if ok do that, if not neutral
                if((j & REGEN_ENABLED_BIT) && (j & READY_TO_REGEN_BIT)){ //Checking if regen enabled AND ready to regen
                    FSM[i].NextStates[j] = REGEN; //Blended braking
                }else{
                    FSM[i].NextStates[j] = NEUTRAL;
                }
            }else if((j & REGEN_ENABLED_BIT) && (j & READY_TO_REGEN_BIT) && (j & REGEN_BUTTON_BIT) && !(j & CRUISE_CONTROL_BUTTON_BIT)){ //Checking if regen enabled AND regen button AND ready to regen AND NOT cruising
                //Regen
                FSM[i].NextStates[j] = REGEN;
            }else if((j & CRUISE_CONTROL_BUTTON_BIT) && ((j & FORWARD_BIT) == 1) && (j & REGEN_ENABLED_BIT)){ //If we tryna cruise control AND forward AND Regen ok
                //Cruise control
                FSM[i].NextStates[j] = CRUISE_CONTROL;
            }else if((j & 0x3) == FORWARD_BIT){ //If we are tryna go forward
                //Forward drive, check if we are going from rev to forward as well
                if(i == REVERSE_DRIVE){
                    //If we are going from rev to forward, go to neutral first
                    FSM[i].NextStates[j] = NEUTRAL;
                }else{
                    FSM[i].NextStates[j] = FORWARD_DRIVE;
                }
            }else if((j & 0x3) == REVERSE_BIT){ //If we are tryna go reverse
                //Reverse drive, check if we are going from forward to rev as well
                if(i == FORWARD_DRIVE){
                    //If we are going from forward to rev, go to neutral first
                    FSM[i].NextStates[j] = NEUTRAL;
                }else{
                    FSM[i].NextStates[j] = REVERSE_DRIVE;
                }
            }else if ((j & 0x3) == NOT_READY_BITS){
                //Not ready
                FSM[i].NextStates[j] = CAR_NOT_READY;
            }
            else{
                //Neutral
                FSM[i].NextStates[j] = NEUTRAL;
            }
        }
    }

    //SPECIFIC STATE SETUPS (none atm)
}


void sendMotorDriveCommand(float velocitySetpoint, float currentSetpoint){
    memset(&driveCmd.data[0], 0, sizeof(driveCmd.data)); //Clear it 

    memcpy(&driveCmd.data[0], &velocitySetpoint, sizeof(float)); //Set velocity setpoint
    memcpy(&driveCmd.data[4], &currentSetpoint, sizeof(float)); //Set current setpoint

    #ifndef USING_PROFINITY
    CANbus_Send(driveCmd, true, MOTORCAN);
    #endif

    SendCarCAN_Put(driveCmd); // Send the drive command to the car CAN bus for telemetry
    
}

void sendMotorPowerCommand(float powerSetpoint){
    memset(&powerCmd.data[0], 0, sizeof(powerCmd.data)); //Clear it 

    memcpy(&powerCmd.data[0], &powerSetpoint, sizeof(float)); //Set power setpoint

    #ifndef USING_PROFINITY
    CANbus_Send(powerCmd, true, MOTORCAN);
    #endif
    SendCarCAN_Put(powerCmd); // Send the power command to the car CAN bus for telemetry
}

bool readyToRoll(){ //Car can escape not ready state, this is all of our checks for recoverable faults and init states, but init stats will have associated flags
    
    //Check all of our status bits here for other random faults 
    return accelPedalPercent < ACCEL_PEDAL_THRESHOLD && 1 && 1; //Add other checks here
}

OS_ERR checkForAllFaults(){
    OS_ERR err = MotorStatus_Wait(BPS_SAFE | BPS_CHECKED | MOTOR_SAFE_TO_RUN, !OS_FLAG_BLOCKING);
    return err;
}

void checkWatchDogs(){
    OS_ERR err;


}

//Methods for handling all the FSM states

void handleFSMNotReadyState(){
    //Starting up, should so something here but not sure, over here make sure ignition good prolly and go back to neutral if motor ok
    handleFSMNeutralState();
    currentState = readyToRoll() ? FSM[NEUTRAL] : FSM[CAR_NOT_READY];
    return;
}

void handleFSMForwardDriveState(){
    sendMotorDriveCommand(MAX_VELOCITY, accelPedalPercent);
    return;
}
void handleFSMNeutralState(){
    sendMotorDriveCommand(0, 0);
    return;
}

void handleFSMReverseDriveState(){
    sendMotorDriveCommand(MAX_VELOCITY, -accelPedalPercent);
    return;
}

void handleFSMRegenState(){
    float regenCurrent = 100.0f;
    sendMotorDriveCommand(0, regenCurrent);
    return;
}

void handleFSMCruiseControlState(){

    float cruiseControlPercent = accelPedalPercent; //Set this to some actual velocity later, when button pressed
    sendMotorDriveCommand(MAX_VELOCITY, cruiseControlPercent);
    return;
}

void handleFSMDisabledState(){
    handleFSMNeutralState(); //Just stop motor and current and throw/handle faults
    OS_FLAGS flags = MotorStatus_GetBits(); //Do something with this later...
    return;
}

void handleFSMInitState(){
    //Was going to init CANBUS but we shall see
    //Relavant CAN messages to read
    carCANFilterList[0] = IO_STATE;
    //...Finish this in a bit


    for(int i = 0; i < NUM_CARCAN_FILTERS; i++){
        CAR_MSGS[i].ID = carCANFilterList[i]; //Shouldn't matter if all the IDs aren't filled
    }
    
    initFSM();

    currentState = FSM[CAR_NOT_READY]; //Go to not ready after init
    return;
}

uint8_t generateCustomBitfield(BitfieldInputs_t[] inputs){
    //Inputs are in order of most significant bit to least significant bit
    uint8_t bitfield = 0;
    for(int i = 0; i < sizeof(inputs)/sizeof(inputs[0]); i++){
        bitfield |= inputs[i];
    }
    return bitfield;
}

uint8_t getControlsBitfield(){
    //Return the current state of the Car, this is through CAN commands
    uint8_t status = 0;

     //Update the IO state forceably, will change this later...
    getAndUpdateControlStatus(CAR_MSGS, NUM_CARCAN_FILTERS);

    //Figuring out all the bits for the bitfield, this is only the logic, we have already gotten the data
    for(int i = 0; i < NUM_CARCAN_FILTERS; i++){
        switch(CAR_MSGS[i].CANMessage.ID){
            case IO_STATE: //Only thing we need for daybreak rn, has pedals info + gears
                if(gear == DASH_FWD){
                    status |= FORWARD_BIT;
                } else if(gear == DASH_REV){
                    status |= REVERSE_BIT;
                } else if(gear == DASH_NEU){
                    status |= NEUTRAL_BIT;
                } else if(gear == DASH_INIT){
                    status |= NOT_READY_BITS;
                }else{
                    assertSendTritiumError(C_ERR_STR_GEAR_FAULT);
                    break;
                }
 

                if(brakePedalPercent >= thresholdBrake){
                    status |= BRAKING_BIT;
                    isBrakeOn = true;
                }else {
                    isBrakeOn = false;
                }
                break;
            default:
                //Do nothing for now
                break;
        }
    }

    if(status & BRAKING_BIT){ //If braking, thresh for braking goes down
        thresholdBrake = BRAKE_THRESH_HYST;
    } else {
        thresholdBrake = BRAKE_THRESH;
    }

    return status;
}

//For CAN Architecture
void getAndUpdateControlStatus(FSMCANDATA_t * messages, uint8_t numMessages){ //This is for getting the data, the logic will happen seperately
    //Reading all of the relevant data on the BUS
    for(int k = 0; k < CAN_MSG_BUFFER_SIZE; k++){
        CANDATA_t dataBuf = {0};
        CANbus_Read(&dataBuf, false, CARCAN);
        //Have a list of IDs and update the associated one? Needs timestamp as well for watchdogs, have a thread calling this constantly

        for(int i = 0; i < numMessages; i++){
            if(dataBuf.ID == messages[i].CANMessage.ID){
                memcpy((void *)&messages[i].CANMessage, (void *)&dataBuf, sizeof(CANDATA_t));
                //messages[i].timestamp = OSTimeGet(NULL); //Get current time

                // switch(messages[i].CANMessage.ID){
                //     case: IO_STATE:
                //         //Update brake and accel percent
                //         brakePedalPercent = ((uint8_t*)messages[i].CANMessage.data)[0];
                //         accelPedalPercent = ((uint8_t*)messages[i].CANMessage.data)[1];

                        // brakePedalPercent = mapToPercent(brakePedalPercent, BRAKE_PEDAL_MIN, BRAKE_PEDAL_MAX, 0, 100);
                //         accelPedalPercent = mapToPercent(accelPedalPercent, ACCEL_PEDAL_MIN, ACCEL_PEDAL_MAX, 0, 100);
                //         //Gear is bits 3 and 4 of byte 3
                //         gear = (((uint8_t*)messages[i].CANMessage.data)[2] >> 3) & 0x03; 
                //         break;
                //     default:
                //         break;
                //         //Do nothing, need to add other ids later
                // }

                readInputs(); //Ignoring the CAN for now and just reading inputs
                messages[i].inputBit = NULL;
            }
        }
    }
    
}



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

    isBrakeOn ? Lights_Write(BRAKE_LIGHT, ON) : Lights_Write(BRAKE_LIGHT, OFF); //Lights sep from the the set brake display??
}

/**
 * @brief Reads inputs from the system
 */
static void readInputs() {
    // brakePedalPercent = Pedals_Read(BRAKE);
    brakePedalPercent = mapToPercent(Pedals_Read(BRAKE), ACCEL_PEDAL_THRESHOLD, PEDAL_MAX, CURRENT_SP_MIN, 100);
    accelPedalPercent = mapToPercent(Pedals_Read(ACCELERATOR), ACCEL_PEDAL_THRESHOLD, PEDAL_MAX, CURRENT_SP_MIN, 100);
    CANDATA_t rawPedalmv = {
        .ID = PEDALS_RAW_VOLTAGE,
        .idx = 0,
        .data = {0}
    };

    ((int16_t*)rawPedalmv.data)[0] = Pedals_rawVoltage(BRAKE);
    ((int16_t*)rawPedalmv.data)[1] = Pedals_rawVoltage(ACCELERATOR);
    SendCarCAN_Put(rawPedalmv);

    // // // Brake hysteresis
    // if (brakePedalPercent >= BRAKE_PRESSED_THRESHOLD) {
    //     isBrakeOn = true;
    //     Lights_Write(BRAKE_LIGHT, ON);
    // }
    // else if (brakePedalPercent <= BRAKE_UNPRESSED_THRESHOLD) {
    //     isBrakeOn = false;
    //     Lights_Write(BRAKE_LIGHT, OFF);
    // }
    gear = getGear(GEAR_USE_OS_DELAY);

    // // Check for gear fault
    // if (gear == DASH_GEAR_FAULT_ERROR) {
    //     // Fault behavior
    //     if (gearFaultCnt > GEAR_FAULT_THRESHOLD)
    //         assertSendTritiumError(C_ERR_STR_GEAR_FAULT);
    //     else
    //         gearFaultCnt++;
    // } else {
    //     gearFaultCnt = 0;
    // }
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


    // Accelerator is pulled to GND and then inverted in code so when nothing is plugged in accel defaults to 100%
    // In order to spin the motor the accelerator needs to return to a safe value
    // static bool accelerator_reset = false;
    // uint8_t resetAccelPercent = 0;
    // uint8_t maxCurrentPercentage = 100;

    // By default assume we are below the motor swoc threshold at startup
    MotorStatus_ModifyBits(MOTOR_SWOC_THRESHOLD, true, false);

    //Not done yet need to update variables and make callbacks...

    //This is from ADC what do I check?
    OSTmrCreate(
        &canWatchTimer, "Pedals Input",
        CAN_WATCH_TMR_DLY_TMR_TS, // Initial delay equal to the period since 0 doesn't seem to work
        CAN_WATCH_TMR_DLY_TMR_TS, OS_OPT_TMR_PERIODIC, callbackCANWatchdog, NULL, &err);
    assertOSError(err);

    // Start CAN Watchdog timer
    OSTmrStart(&canWatchTimer, &err);
    assertOSError(err);

    //Is something else already checking this?
    OSTmrCreate(
        &prechargeCanWatchTimer, "BPS CAN",
        CAN_WATCH_TMR_DLY_TMR_TS, // Initial delay equal to the period since 0 doesn't seem to work
        CAN_WATCH_TMR_DLY_TMR_TS, OS_OPT_TMR_PERIODIC, callbackCANWatchdog, NULL, &err);
    assertOSError(err);
    

    OSTmrStart(&prechargeCanWatchTimer, &err);
    assertOSError(err);


    while (1) {
#ifdef TASK_PROFILER
        DebugIO_Toggle(SEND_TRITIUM_PIN);
#endif
        // readInputs(); // read inputs from the system
        runFSM(); // run the FSM to update the velocity and current setpoints
        updateDisplayState();

        // Check that motor is ready to run
        // err = MotorStatus_Wait(BPS_SAFE | BPS_CHECKED | MOTOR_SAFE_TO_RUN, !OS_FLAG_BLOCKING);

        // // if you return OS_ERR_PEND_WOULD_BLOCK, one of the bits are not sent, and would've blocked
        // // If the error is ERR_NONE, assertOSError returns without asserting an error
        // if (err != OS_ERR_PEND_WOULD_BLOCK) {
        //     assertOSError(err);
        // }

        // memset(&motorSafeCmd.data, 0, sizeof(motorSafeCmd.data));
        // // All bits are set
        // if (err == OS_ERR_NONE) {
        //     // CAN message for setpoint of bus current percent
        //     memcpy(&powerCmd.data[4], &busCurrentSetPoint, sizeof(float));
        //     // If we're using the profinity software don't set the power here
// #ifndef USING_PROFINITY
//             CANbus_Send(powerCmd, CAN_BLOCKING, MOTORCAN);
// #endif
            // The accelerator at some point has been lowered to a safe value
            // if(accelerator_reset){
            //     resetAccelPercent = accelPedalPercent;
            // }
            // // The accelerator is now at a safe value
            // else if(!accelerator_reset && accelPedalPercent <= ACCCEL_PEDAL_RESET_THRESHOLD){
            //     accelerator_reset = true;
            //     resetAccelPercent = accelPedalPercent;
            // }
            // // The accelerator has ever been set to a safe value
            // else{
            //     resetAccelPercent = 0;
            // }

        //     err = MotorStatus_Wait(MOTOR_SWOC_THRESHOLD, !OS_FLAG_BLOCKING);
        //     maxCurrentPercentage = (err == OS_ERR_NONE) ? SWOC_CURRENT_SP_MAX : CURRENT_SP_MAX;
        //     // The motor is going fast enough where you want to scale the max current setpoint to prevent SWOC

        //     switch (gear) {
        //         case DASH_FWD:
        //             velocitySetpoint = MAX_VELOCITY;
        //             currentSetpoint = isBrakeOn ? 0 : (mapToPercent(resetAccelPercent, 
        //                                                             ACCEL_PEDAL_THRESHOLD, 
        //                                                             PEDAL_MAX, 
        //                                                             CURRENT_SP_MIN, 
        //                                                             maxCurrentPercentage));                 
        //             break;

        //         case DASH_NEU:
        //             velocitySetpoint = 0.0f;
        //             currentSetpoint = 0.0f;
        //             break;

        //         case DASH_REV:
        //             velocitySetpoint = -MAX_VELOCITY;
        //             currentSetpoint = isBrakeOn ? 0 : (mapToPercent(resetAccelPercent, 
        //                                                             ACCEL_PEDAL_THRESHOLD, 
        //                                                             PEDAL_MAX, 
        //                                                             CURRENT_SP_MIN, 
        //                                                             maxCurrentPercentage));   
        //             break;

        //         default:
        //             assertSendTritiumError(C_ERR_STR_GEAR_FAULT);
        //             break;
        //     }
        //     // motorSafeCmd.data[0] |= 0x01; // Set motor safe to run to true
        // }

        // // Motor is not safe to run so velocitySetpoint and currentSetpoint are set to 0
        // else {
        //     velocitySetpoint = 0.0f;
        //     currentSetpoint = 0.0f;
        //     motorSafeCmd.data[0] = 0; // Set motor safe to run to false
        // }
        // memcpy(&driveCmd.data[4], &currentSetpoint, sizeof(float));
        // memcpy(&driveCmd.data[0], &velocitySetpoint, sizeof(float));

        // Set motor fault as 0
        // The motor controller will error if 2 different sources are sending drive commands, so if
        // profinity is plugged in, don't send drive command
// #ifndef USING_PROFINITY
//         // Drive command must be sent every 250ms or the motor will return to neutral
//         CANbus_Send(driveCmd, CAN_BLOCKING, MOTORCAN);
// #endif
//         SendCarCAN_Put(motorSafeCmd); // Send the motor safe command
//         SendCarCAN_Put(driveCmd); // Send the drive command to the car CAN bus for telemetry

        // Delay of FSM_PERIOD ms
        OSTimeDlyHMSM(0, 0, 0, FSM_PERIOD, OS_OPT_TIME_HMSM_STRICT, &err);
        assertOSError(err);
#ifdef TASK_PROFILER
        DebugIO_Toggle(SEND_TRITIUM_PIN);
#endif
    }
}

static void assertSendTritiumError(controls_error_e sterr) {
    // switch (sterr) {
    // case C_ERR_NONE:
    //     break;
    // case C_ERR_STR_GENERIC:
    // case C_ERR_STR_GEAR_FAULT:
    //     throwTaskError(sterr, false, NULL, OPT_LOCK_SCHED, OPT_NONRECOV, CAN_NONE_BPS);
    //     break;
    // default:
    //     // Critical failure, we have a non sendtritium error in send tritium somehow
    //     throwTaskError(C_ERR_ILLEGAL_ERROR, false, NULL, OPT_LOCK_SCHED, OPT_NONRECOV, CAN_NONE_BPS);
    //     break;
    // }
    throwTaskError(sterr);
}


//Lin map, display, task finish and verify, fault check from helper, do some SWOC logic in fault thread, watch dogs...
//make faults tuff and unify all of em 