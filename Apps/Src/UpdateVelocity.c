#include "SendTritium.h"
#include "CarState.h"
#include "UpdateVelocity.h"
#define shift 27
#define first -1
#define second 134

extern const float pedalToPercent[];

// As of 03/13/2021 the function just returns a 1 to 1 conversion
static float convertPedaltoMotorPercent(uint8_t pedalPercentage) {
    return pedalToPercent[pedalPercentage];
}

void Task_UpdateVelocity(void* p_arg) {
    
    car_state_t *car_state = (car_state_t *) p_arg;

    OS_ERR err;

    //Set desired velocity to 0
    car_state->DesiredVelocity = 0;
    while(1) {
        // Regen or cruise control mode require regen to be enabled and no pedals being touched
        if(car_state->IsRegenBrakingAllowed && car_state->BrakePedalPercent < 5 && car_state->AccelPedalPercent < 5) {
            // If in active cruise mode
            if(car_state->CruiseControlEnable && car_state->CRSet == CRUISE) {
                car_state->DesiredVelocity = car_state->CruiseControlVelocity;
                car_state->DesiredMotorCurrent = 1.0f;
            } else if(car_state->CRSet == REGEN) { // If in regen mode
                switch(car_state->RegenButtonMode) {
                    case REGEN_OFF:
                        car_state->CRSet = ACCEL;
                        car_state->RegenBrakeRate = 4;
                        break;
                    case RATE3:
                        car_state->RegenBrakeRate = 3;
                        break;
                    case RATE2:
                        car_state->RegenBrakeRate = 2;
                        break;
                    case RATE1:
                        car_state->RegenBrakeRate = 1;
                        break;
                }
                car_state->DesiredVelocity = (car_state->CurrentVelocity*car_state->RegenBrakeRate)/4;
                car_state->DesiredMotorCurrent = 1.0f;
            }
        } else { // Not in regen or cruise control mode
            car_state->CRSet = ACCEL;
            car_state->CruiseControlEnable = OFF;
        }

        if(car_state->CRSet == ACCEL) {
            car_state->DesiredVelocity = MAX_VELOCITY;
            car_state->DesiredMotorCurrent = convertPedaltoMotorPercent(car_state->AccelPedalPercent);
        }

        // Delay of few milliseconds (10)
        OSTimeDlyHMSM (0, 0, 0, 10, OS_OPT_TIME_HMSM_STRICT, &err);
        
        if(err != OS_ERR_NONE){
            car_state->ErrorCode.UpdateVelocityErr = ON;
        }
    }
}

