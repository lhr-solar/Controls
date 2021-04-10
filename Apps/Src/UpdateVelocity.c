#include "SendTritium.h"
#include "CarState.h"
#include "UpdateVelocity.h"

// As of 03/13/2021 the function just returns a 1 to 1 conversion
static float convertPedaltoMotorPercent(uint8_t pedalPercentage){
    return ((float) pedalPercentage)/100;
}

void Task_UpdateVelocity(void* p_arg){
    
    car_state_t *car_state = (car_state_t *) p_arg;

    OS_ERR err;

    //Set desired velocity to 0
    car_state->DesiredVelocity = 0;

    while(1){
        //Read global pedal percentage
        uint8_t pedalPercentage = car_state->AccelPedalPercent;

        if(car_state->CruiseControlEnable && car_state -> CruiseControlSet){
            car_state->DesiredVelocity = car_state->CruiseControlVelocity;
            car_state->DesiredMotorCurrent = 1.0f;
        }else{
            car_state->DesiredVelocity = MAX_VELOCITY;
            car_state->DesiredMotorCurrent = convertPedaltoMotorPercent(pedalPercentage);
        }        
        // Delay of few milliseconds (10)
        OSTimeDlyHMSM (0, 0, 0, 10, OS_OPT_TIME_HMSM_STRICT, &err);
    }
}

