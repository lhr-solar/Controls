/**
 * @file UpdateVelocity.c
 * @author Nathaniel Delgado (nathaniel.delgado@utexas.edu)
 * @brief Continually reads inputs from the system to decide how the motor acts
 * @version 0.1
 * @date 2022-09-17
 * 
 * @copyright Copyright (c) 2022
 * 
 */

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
// range of regen working in one-pedal drive
#define REGEN_RANGE (NEUTRAL_PEDALS_PERCENT - UNTOUCH_PEDALS_PERCENT_ACCEL)

extern const float pedalToPercent[];
// static float convertPedaltoMotorPercent(uint8_t pedalPercentage){
//     if(pedalPercentage > 100){
//         return 1.0f;
//     }
//     return pedalToPercent[pedalPercentage];
// }

// Convert a float velocity in meters per second to a desired RPM
static float velocity_to_rpm(float velocity) {
    float velocity_mpm = velocity * 60.0f; // velocity in meters per minute
    const float circumference = WHEEL_DIAMETER * M_PI;
    float wheel_rpm = velocity_mpm / circumference;
    return wheel_rpm;
}

// Macros for states
#define IDLE 0
#define ACCEL 1

void Task_UpdateVelocity(void *p_arg){
    OS_ERR err;

    uint8_t state = IDLE;
    
    while(1){
        // Get inputs to decide actions
        // uint8_t accelPedalPercent = Pedals_Read(ACCELERATOR);
        uint8_t brakePedalPercent = Pedals_Read(BRAKE);

        // Start of with idle parameters
        float desiredVelocity = 0;
        float desiredMotorCurrent = 0;

        // Do output based on state and decide on the next state
        switch(state){
            case IDLE:
                if(Contactors_Get(MOTOR_CONTACTOR)){
                    MotorController_Drive(velocity_to_rpm(desiredVelocity), desiredMotorCurrent);
                }

                // Decide on the next state
                if(brakePedalPercent > UNTOUCH_PEDALS_PERCENT_BRAKE){
                    state = IDLE;
                }else if(!(Switches_Read(FOR_SW) || Switches_Read(REV_SW))){
                    state = IDLE;
                }else if(Switches_Read(FOR_SW) || Switches_Read(REV_SW)){
                    state = ACCEL;
                }

                break;
            case ACCEL:
                if(Switches_Read(FOR_SW)){
                    desiredVelocity = MAX_VELOCITY;
                } else if (Switches_Read(REV_SW)) {
                    desiredVelocity = -MAX_VELOCITY;
                }

                if(Contactors_Get(MOTOR_CONTACTOR)){
                    MotorController_Drive(velocity_to_rpm(desiredVelocity), desiredMotorCurrent);
                }

                // Decide on the next state
                if(brakePedalPercent > UNTOUCH_PEDALS_PERCENT_BRAKE){
                    state = IDLE;
                }else if(!(Switches_Read(FOR_SW) || Switches_Read(REV_SW))){
                    state = IDLE;
                }else if(Switches_Read(FOR_SW) || Switches_Read(REV_SW)){
                    state = ACCEL;
                }
                break;
        }

        // Set brake lights
        if(brakePedalPercent <= UNTOUCH_PEDALS_PERCENT_BRAKE){
            Lights_Set(BrakeLight, ON);
        }
        else{
            Lights_Set(BrakeLight, OFF);
        }

        // Delay of few milliseconds (100)
        OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &err);
        if (err != OS_ERR_NONE){
            assertOSError(OS_UPDATE_VEL_LOC, err);
        }
    }
}
