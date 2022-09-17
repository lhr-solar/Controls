#include "UpdateVelocity.h"
#include "Pedals.h"
#include "Tasks.h"
#include "Minions.h"
#include "MotorController.h"
#include "Contactors.h"
#include <math.h>

// REGEN_CURRENT AS A PERCENTAGE (DECIMAL NUMBER) OF MAX CURRENT
#define REGEN_CURRENT 0.5f
// THRESHOLD VELOCITY IN M/S
#define THRESHOLD_VEL 1.0f
// UNTOUCH PEDAL PERCENTS AS A PERCENTAGE OF MAX PEDAL POSITION
#define UNTOUCH_PEDALS_PERCENT_ACCEL 0 //tuning constant for the accelerator pedal
#define UNTOUCH_PEDALS_PERCENT_BRAKE UNTOUCH_PEDALS_PERCENT //tuning constant for the brake pedal
#define UNTOUCH_PEDALS_PERCENT 5 //generic tuning constant for both pedals (assuming we want to use the same one for both)

// percent of pedal to count as neutral (to add support for one-pedal drive)
#define NEUTRAL_PEDALS_PERCENT 25
#define REGEN_RANGE (NEUTRAL_PEDALS_PERCENT - UNTOUCH_PEDALS_PERCENT_ACCEL)

extern const float pedalToPercent[];

// Shared with ReadSwitches.c
bool UpdateVel_ToggleCruise = false;

static float convertPedaltoMotorPercent(uint8_t pedalPercentage)
{
    if(pedalPercentage > 100){
        return 1.0f;
    }
    return pedalToPercent[pedalPercentage];
}

// Convert a float velocity in meters per second to a desired RPM
static float velocity_to_rpm(float velocity) {
    float velocity_mpm = velocity * 60.0f; // velocity in meters per minute
    const float circumference = WHEEL_DIAMETER * M_PI;
    float wheel_rpm = velocity_mpm / circumference;
    return wheel_rpm;
}

void Task_UpdateVelocity(void *p_arg){
    OS_ERR err;

    static float cruiseSpeed = 0;       //cruising speed
    static float cruiseRPM = 0;
    static State cruiseState = OFF;     //whether cruise is enabled

    float desiredVelocity = 0;          //reset desired velocity to 0
    float desiredMotorCurrent = 0;      //reset desired current to 0
    State RegenState = OFF;             //reset regen state to 0. This state var denotes whether or not we are in the regen brake mode

    while(1){
        // Get preliminary data
        uint8_t accelPedalPercent = Pedals_Read(ACCELERATOR);
        uint8_t brakePedalPercent = Pedals_Read(BRAKE);
        State RegenSwitchState = Switches_Read(REGEN_SW);
        RegenState = ((RegenSwitchState == ON) && (RegenEnable == ON)); //AND driver regen enable and system regen enable together to decide whether we are in regen brake mode

        // Calculate cruise state
        if(UpdateVel_ToggleCruise){         // if a rising edge is detected by ReadSwitches.c, toggle the cruise state
            cruiseState = !cruiseState;
        }
        if(cruiseSpeed < THRESHOLD_VEL){    // if the cruiseSpeed is below the threshold velocity, make the don't go into cruise control
            cruiseState = OFF;              // This is we don't want to have the motor stay at it's speed if it doesn't even move
        }
        UpdateVel_ToggleCruise = false;

        //record speed when we're not already cruising
        if(!cruiseState){
            cruiseSpeed = MotorController_ReadVelocity();
            cruiseRPM = MotorController_ReadRPM();
        }

        // Set brake lights
        if(brakePedalPercent <= UNTOUCH_PEDALS_PERCENT_BRAKE){ //it is less than since we switched to a button instead of the adc
            Lights_Set(BrakeLight, ON);
        }
        else{
            Lights_Set(BrakeLight, OFF);
        }

        // decide if we are pressing the pedal enough to move foward,
        // pressing the break enough to break,
        // and if we've determined to cruise or not
        // look at FSM to see what's up
        bool b = brakePedalPercent <= UNTOUCH_PEDALS_PERCENT_BRAKE;
        bool p = accelPedalPercent > UNTOUCH_PEDALS_PERCENT_ACCEL;
        bool c = cruiseState;
        if(b){  // idle state
            desiredVelocity = 0; //velocity setpoint becomes 0
            cruiseState = OFF; //turn off cruise
            desiredMotorCurrent = (RegenState == ON) ? REGEN_CURRENT : 0; //either regen brake or 0 current brake
        }else if(!p && !c){ // idle state
            desiredVelocity = 0; //velocity setpoint becomes 0
            cruiseState = OFF; //turn off cruise
            desiredMotorCurrent = (RegenState == ON) ? REGEN_CURRENT : 0; //either regen brake or 0 current brake
        }else if(!p && c){  // cruise state

        }else if(p && !c){  // accel state
            if((RegenState == ON) //we're in regen mode
            && (MotorController_ReadVelocity() > THRESHOLD_VEL)  //we're above the threshold value
            && accelPedalPercent < NEUTRAL_PEDALS_PERCENT //we're in the one-pedal drive range
            ){
            
            desiredVelocity = 0;
            // set regen current based on how much the accel pedal is pressed down
            if (accelPedalPercent > UNTOUCH_PEDALS_PERCENT_ACCEL) {
                desiredMotorCurrent = REGEN_CURRENT * (REGEN_RANGE - (accelPedalPercent - UNTOUCH_PEDALS_PERCENT_ACCEL)) / REGEN_RANGE;
            } else {
                desiredMotorCurrent = REGEN_CURRENT;
            }
        }else if(p && c){   // cruiseaccel state
            
        }else{
            // should not happen; invalid
        }

        if ((Contactors_Get(MOTOR_CONTACTOR)) && ((Switches_Read(FOR_SW) || Switches_Read(REV_SW)))) {
            MotorController_Drive(velocity_to_rpm(desiredVelocity), desiredMotorCurrent);
        }

        // Delay of few milliseconds (100)
        OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &err);

        if (err != OS_ERR_NONE){
            assertOSError(OS_UPDATE_VEL_LOC, err);
        }   
    }
}