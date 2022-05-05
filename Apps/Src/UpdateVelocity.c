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
// UNTOUCH_PEDALS_PERCENT AS A PERCENTAGE OF MAX PEDAL POSITION
#define UNTOUCH_PEDALS_PERCENT 5

// percent of pedal to count as neutral (to add support for one-pedal drive)
#define NEUTRAL_PEDALS_PERCENT 25
#define REGEN_RANGE (NEUTRAL_PEDALS_PERCENT - UNTOUCH_PEDALS_PERCENT)

extern const float pedalToPercent[];

// As of 03/13/2021 the function just returns a 1 to 1 conversion
static float convertPedaltoMotorPercent(uint8_t pedalPercentage)
{
    if(pedalPercentage > 100){
        return 1.0f;
    }
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
    
    float desiredVelocity = 0;
    float desiredMotorCurrent = 0;
    
    while (1)
    {
        uint8_t accelPedalPercent = Pedals_Read(ACCELERATOR);
        uint8_t brakePedalPercent = Pedals_Read(BRAKE);

        // Set brake lights
        if(brakePedalPercent >= UNTOUCH_PEDALS_PERCENT){
            Lights_Set(BrakeLight, ON);
        }
        else{
            Lights_Set(BrakeLight, OFF);
        }
    
        // Deadband comparison
        if(brakePedalPercent >= UNTOUCH_PEDALS_PERCENT){
            desiredVelocity = 0;
            desiredMotorCurrent = Switches_Read(REGEN_SW) ? REGEN_CURRENT : 0;
        } 
        else if(Switches_Read(REGEN_SW) 
                && (MotorController_ReadVelocity() > THRESHOLD_VEL) 
                && RegenEnable
                && accelPedalPercent < NEUTRAL_PEDALS_PERCENT){ 
            
            desiredVelocity = 0;
            // set regen current based on how much the accel pedal is pressed down
            desiredMotorCurrent = REGEN_CURRENT * REGEN_RANGE / (NEUTRAL_PEDALS_PERCENT - accelPedalPercent);
        } 
        else {
            if(Switches_Read(FOR_SW)){
                desiredVelocity = MAX_VELOCITY;
            } else if (Switches_Read(REV_SW)) {
                desiredVelocity = -MAX_VELOCITY;
            }
            // handle one-pedal drive
            uint8_t forwardPercent = 0;
            if (accelPedalPercent > NEUTRAL_PEDALS_PERCENT) {
                forwardPercent = (accelPedalPercent - NEUTRAL_PEDALS_PERCENT) * 100 / (100 - NEUTRAL_PEDALS_PERCENT);
            }
            desiredMotorCurrent = convertPedaltoMotorPercent(forwardPercent);
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
