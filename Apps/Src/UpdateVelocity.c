#include "SendTritium.h"
#include "CarState.h"
#include "UpdateVelocity.h"
#define shift 27
#define first -1
#define second 134

extern const float pedalToPercent[];

// As of 03/13/2021 the function just returns a 1 to 1 conversion
static float convertPedaltoMotorPercent(uint8_t pedalPercentage){
    return pedalToPercent[pedalPercentage];
}

void Task_UpdateVelocity(void* p_arg){
    
    car_state_t *car_state = (car_state_t *) p_arg;

    OS_ERR err;

    //Set desired velocity to 0
    car_state->DesiredVelocity = 0;
    int time =0;
    float velInit=0;
    float initcurrent=0;
    while(1){
        //Read global pedal percentage
        uint8_t pedalPercentage = car_state->AccelPedalPercent;
        // first need to fix the desired Motor current so that when it gets closer to the cruise control velocity it lowers the current needed to get there
        // there should be an error every single time that it doesnt reach that desired velocity 
        float velFin=car_state->CurrentVelocity;
        time+=10;
        float acc= (velFin-velInit)/10; // finds the acceleration - time difference is always going to be 10 milliseconds
        velInit=velFin;
        // you can see that if there is posiitve acceleration the velfinal the next time around should have a higher current velocity whatever that might be 
        // maybe you can check what the current value is and based on that see if it should be increasing 

        if(car_state->CruiseControlEnable && car_state -> CruiseControlSet){
            car_state->DesiredVelocity = car_state->CruiseControlVelocity;
            //32 bits for the whole fixed point - give some wiggle room 
            float current=(float)(((-172>>34)*((time*time)))+((134>>27)*(time))+(1)); // calculated a quadratic equation - used fixed point //first number was -1.0080e-8 and the second number is 9.995577e-7. first part should be 1/ (2^27) and the second number is 134 / (2^27). changed the first part to -172>>34
            if(current<0.4f){
                current=0.4f; //should never be less than .4 
            }
            if(current>=1.f){
                current=.99f;
            }
            car_state->DesiredMotorCurrent = current; //it was at 1.0f - make sure that the number you put in is float 
        }else{
            car_state->DesiredVelocity = MAX_VELOCITY;
            car_state->DesiredMotorCurrent = convertPedaltoMotorPercent(pedalPercentage);
        } 
        //if there is a change in current there needs to be a change in acceleration 
        float finalCurrent = car_state->DesiredMotorCurrent;  
        float changeCurrent=(finalCurrent-initcurrent)/10;
        if(time!=0){
            if(time%2000==0 && ((changeCurrent==0 && acc!=0) || (changeCurrent!=0 && acc==0))){
            //gives an error becauase there is current going through but no acceleration or there is no current but there is acceleration 
             }
        }
        initcurrent=finalCurrent;
        // Delay of few milliseconds (10)
        OSTimeDlyHMSM (0, 0, 0, 10, OS_OPT_TIME_HMSM_STRICT, &err);
    }
}

