#include "UpdateVelocity.h"
#include "Pedals.h"
#include "Tasks.h"
#include "Minions.h"
#include "MotorController.h"
#include <math.h>
#define UNTOUCH_PEDALS_PERCENT 5
#define REGEN_CURRENT 0.5f
//#define UNATTAINABLE_VELOCITY 1000.0f

extern const float pedalToPercent[];

// As of 03/13/2021 the function just returns a 1 to 1 conversion
static float convertPedaltoMotorPercent(uint8_t pedalPercentage)
{
    return pedalToPercent[pedalPercentage];
}

// Convert a float velocity to a desired RPM
static float velocity_to_rpm(float velocity) {
    float velocity_mpm = velocity * 60.0f; // velocity in meters per minute
    const float circumfrence = WHEEL_DIAMETER * M_PI;
    float wheel_rpm = velocity_mpm / circumfrence;
    return wheel_rpm;
}

void Task_UpdateVelocity(void *p_arg)
{
    OS_ERR err;

    MotorController_Init(1.0f);

    static uint8_t prevBrakePedalPercent;

    //Prev refers to it being pressed before
    static State cruzEnablePrev = OFF;
    static State cruzSetPrev = OFF;    

    //State refers to whether we consider the enable and set to be on or off
    static State cruzEnableState = OFF;
    static State cruzSetState = OFF;
    
    float desiredVelocity = 0;
    float desiredMotorCurrent = 0;
    while (1)
    {
        uint8_t accelPedalPercent = Pedals_Read(ACCELERATOR);
        uint8_t brakePedalPercent = Pedals_Read(BRAKE);

        // If the brake is changed, then the brake lights might need to be changed
        if (brakePedalPercent != prevBrakePedalPercent)
        {
            if(brakePedalPercent >= UNTOUCH_PEDALS_PERCENT){
                Lights_Set(BrakeLight, ON);
            }
            else{
                Lights_Set(BrakeLight, OFF);
            }
        }

        prevBrakePedalPercent = brakePedalPercent;

        State regenPressed = Switches_Read(REGEN_SW);

        //Current refers to it currently being pressed
        State cruzEnableCurr = Switches_Read(CRUZ_EN);
        State cruzSetCurr = Switches_Read(CRUZ_ST);

        //The cruzEnableState is toggled on the rising edge of the button press
        if(cruzEnableCurr && !cruzEnablePrev){
            cruzEnablePrev = ON;
            cruzEnableState = (State) ((int)cruzEnableState ^ 1);
        }else if(!cruzEnableCurr){
            cruzEnablePrev = OFF;
        }

        //The cruzSetState is toggled on the rising edge of the button press
        if(cruzSetCurr && !cruzSetPrev){
            cruzSetPrev = ON;
            cruzSetState = (State) ((int)cruzSetState ^ 1);
        }else if(!cruzSetCurr){
            cruzSetPrev = OFF;
        }

        //Regen will be level sensitive meaning the user will regen brake by holding down a button
        if(regenPressed && RegenEnable && (brakePedalPercent < UNTOUCH_PEDALS_PERCENT)){
            desiredVelocity = 0;
            desiredMotorCurrent = REGEN_CURRENT;
        } else if(cruzEnableState && cruzSetState && brakePedalPercent < UNTOUCH_PEDALS_PERCENT 
            && accelPedalPercent < UNTOUCH_PEDALS_PERCENT){
            desiredVelocity = MotorController_ReadVelocity();
            desiredMotorCurrent = 1.0f;
        }
        else if(brakePedalPercent >= UNTOUCH_PEDALS_PERCENT){
            desiredVelocity = 0;
            desiredMotorCurrent = 0;
        }
        else{
            desiredVelocity = MAX_VELOCITY;
            desiredMotorCurrent = convertPedaltoMotorPercent(accelPedalPercent);
        }

        MotorController_Drive(velocity_to_rpm(desiredVelocity), desiredMotorCurrent);

        // Delay of few milliseconds (10)
        OSTimeDlyHMSM(0, 0, 0, 10, OS_OPT_TIME_HMSM_STRICT, &err);

        if (err != OS_ERR_NONE){
            assertOSError(OS_UPDATE_VEL_LOC, err);
        }
    }
}