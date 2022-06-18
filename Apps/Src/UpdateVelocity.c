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
#define UNTOUCH_PEDALS_PERCENT_ACCEL UNTOUCH_PEDALS_PERCENT //tuning constant for the accelerator pedal
#define UNTOUCH_PEDALS_PERCENT_BRAKE UNTOUCH_PEDALS_PERCENT //tuning constant for the brake pedal
#define UNTOUCH_PEDALS_PERCENT 5 //generic tuning constant for both pedals (assuming we want to use the same one for both)

// percent of pedal to count as neutral (to add support for one-pedal drive)
#define NEUTRAL_PEDALS_PERCENT 25
#define REGEN_RANGE (NEUTRAL_PEDALS_PERCENT - UNTOUCH_PEDALS_PERCENT_ACCEL)

static float cruiseSpeed = 0; //cruising speed
static State cruiseEnable = OFF; //whether cruise is enabled
static State cruiseSet = OFF; //whether cruise is set

static State currEnable = OFF; //state variable for edge detector Cruise Enable Button
static State currSet = OFF; //state variable for edge detector on Cruise Set Button
static State prevEnable = OFF; //state variable for edge detector on Cruise Enable button
static State prevSet = OFF; //state variable for edge detector on Cruise Set button
extern const float pedalToPercent[];


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
    const float circumfrence = WHEEL_DIAMETER * M_PI;
    float wheel_rpm = velocity_mpm / circumfrence;
    return wheel_rpm;
}

void Task_UpdateVelocity(void *p_arg)
{
    OS_ERR err;
    
    float desiredVelocity = 0;
    float desiredMotorCurrent = 0;
    
    while (1)
    {
        uint8_t accelPedalPercent = Pedals_Read(ACCELERATOR);
        uint8_t brakePedalPercent = Pedals_Read(BRAKE);

        //Edge detector to toggle the cruise enable on only the rising edge
        prevEnable = currEnable; //store old state in prev
        currEnable = Switches_Read(CRUZ_EN); //read in current state
        if((prevEnable == OFF) && (currEnable == ON)){ //we went from off to on, so we should toggle it between on and off
            cruiseEnable ^= 1; 
        }
        //no need for an else block, all following logic is only dependent on the cruiseEnable value

        //Edge detector to toggle the cruise set on only the rising edge
        prevSet = currSet; // store old state in prev
        currSet = Switches_Read(CRUZ_ST); //read in the current state
        if((prevSet == OFF) && (currSet == ON)){ //we went from off to on, so we should toggle it between on and off
            cruiseSet ^= 1;
        }
        //no need for an else block, all following logic is only dependent on the cruiseSet value

        if(cruiseEnable==OFF){ //cruise Enable being off overrides cruiseSet edge detector
            cruiseSet = OFF;
        }

        State cruiseState = cruiseSet & cruiseEnable; //if both are true, then we are in cruise mode, if not, we are in normal drive
        
        //record speed when we're not already cruising
        if(!cruiseState){
            cruiseSpeed = MotorController_ReadVelocity();
        }
        // Set brake lights
        if(brakePedalPercent >= UNTOUCH_PEDALS_PERCENT_BRAKE){
            Lights_Set(BrakeLight, ON);
        }
        else{
            Lights_Set(BrakeLight, OFF);
        }
        

        // Deadband comparison
        if(brakePedalPercent >= UNTOUCH_PEDALS_PERCENT_BRAKE){ //mech brake is pushed down
            desiredVelocity = 0; //velocity setpoint becomes 0
            cruiseSet = OFF; //unset cruise, but don't disable cruise system
            cruiseState = OFF; //turn off the cruise state
            cruiseSpeed = 0; //set the cruise velocity to 0
            desiredMotorCurrent = (Switches_Read(REGEN_SW) && RegenEnable) ? REGEN_CURRENT : 0; //either regen brake or 0 current brake
        } 
        else if( //One-pedal regen drive case
                Switches_Read(REGEN_SW) //regen switch is pressed
                && (MotorController_ReadVelocity() > THRESHOLD_VEL)  //we're above the threshold value
                && RegenEnable //regen is allowed
                && accelPedalPercent < NEUTRAL_PEDALS_PERCENT //we're in the one-pedal drive range
                && (cruiseState != ON) //we're not in cruise mode
            ){ 
            
            desiredVelocity = 0;
            // set regen current based on how much the accel pedal is pressed down
            if (accelPedalPercent > UNTOUCH_PEDALS_PERCENT_ACCEL) {
                desiredMotorCurrent = REGEN_CURRENT * (REGEN_RANGE - (accelPedalPercent - UNTOUCH_PEDALS_PERCENT_ACCEL)) / REGEN_RANGE;
            } else {
                desiredMotorCurrent = REGEN_CURRENT;
            }
        } 
        else {
            if(Switches_Read(FOR_SW)){
                desiredVelocity = ((cruiseState==ON) ? cruiseSpeed : MAX_VELOCITY); //we're in cruise mode so use the recorded cruise velocity
            } else if (Switches_Read(REV_SW)) {
                desiredVelocity = -MAX_VELOCITY;
            }
            
            //handle neutral range due to one-pedal drive above
            uint8_t forwardPercent = 0;
            if (accelPedalPercent > NEUTRAL_PEDALS_PERCENT) {
                forwardPercent = (accelPedalPercent - NEUTRAL_PEDALS_PERCENT) * 100 / (100 - NEUTRAL_PEDALS_PERCENT);
            }
            
            if((cruiseState == ON)&&(Switches_Read(FOR_SW))){ //we're in cruise mode so use total current to hit cruise velocity
                desiredMotorCurrent = (float) 1.0;
            } else {
                desiredMotorCurrent = convertPedaltoMotorPercent(forwardPercent);
            }

        }

        if (Contactors_Get(MOTOR_CONTACTOR) == ON && (Switches_Read(FOR_SW) || Switches_Read(REV_SW))) {
            MotorController_Drive(velocity_to_rpm(desiredVelocity), desiredMotorCurrent);
        }

        // Delay of few milliseconds (100)
        OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &err);

        if (err != OS_ERR_NONE){
            assertOSError(OS_UPDATE_VEL_LOC, err);
        }
    }
}
