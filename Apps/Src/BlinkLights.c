#include "Tasks.h"
#include "Lights.h"

/**
* @brief This thread controls the blinking of the hazard and indicator lights at the regulation specified frequency. 
**/

void Task_BlinkLight(void* p_arg){
    //get necessary state pointers
    car_state_t *car_state = (car_state_t*) p_arg;
    blinker_states_t *blinkerStates = &(car_state -> BlinkerStates);
    light_switches_t *storedLightSwitches = &(car_state -> SwitchStates.lightSwitches);
    CPU_TS ts;
    OS_ERR err;

    while(1){
        //wait for blinkLight to get signaled. TODO: Does this thread even need a semaphore to signal it? Or can it just be spawned and allowed to run
        OSSemPend( 
            &BlinkLight_Sem4,
            0,
            OS_OPT_PEND_BLOCKING,
            &ts,
            &err
        );    

        //Read the storedLightSwitches and toggle the blinkerSwitches that are on. If else, set to 0. At least one is on.

        if(storedLightSwitches->HZD_SW = ON){ //if the hazards are on, both lights need to get toggled
            blinkerStates->LT = OFF;
            blinkerStates->RT = OFF;
            switch(blinkerStates->HZD){
                case OFF:
                    blinkerStates->HZD = ON;
                    break;
                case ON:
                    blinkerStates->HZD = OFF;
                    break;
            }
        } else if (storedLightSwitches->LEFT_SW){
            //turn off right switch and toggle the left
            blinkerStates->RT = OFF;
            blinkerStates->HZD = OFF;
            switch(blinkerStates->LT){
                case OFF:
                    blinkerStates->LT = ON;
                    break;
                case ON:
                    blinkerStates->LT = OFF;
                    break;
            }
        } else if (storedLightSwitches->RIGHT_SW = ON){
            //turn off the left switch and toggle the right
            blinkerStates->LT = OFF;
            blinkerStates->HZD = OFF;
            switch(blinkerStates->RT){
                case OFF:
                    blinkerStates->RT = ON;
                    break;
                case ON:
                    blinkerStates->RT = OFF;
                    break;
            }
        } else {
            //turn all states off
            blinkerStates->HZD = OFF;
            blinkerStates->LT = OFF;
            blinkerStates->RT = OFF;
        }
        
        //set the lights as defined by the toggled states here.
        if(blinkerStates->HZD = ON){ //Hazard on cycle
            Lights_Set(LEFT_BLINK,ON);
            Lights_Set(RIGHT_BLINK,ON);
        } else if (blinkerStates->LT = ON){ //Left Blink on cycle
            Lights_Set(LEFT_BLINK,ON);
            Lights_Set(RIGHT_BLINK,OFF);
        } else if (blinkerStates->RT = ON){ //Right Blink on cycle
            Lights_Set(LEFT_BLINK,OFF);
            Lights_Set(RIGHT_BLINK,ON);
        } else { //Off cycle
            Lights_Set(LEFT_BLINK,OFF);
            Lights_Set(RIGHT_BLINK,OFF);
        } 


        //lock thread to run at 90ish times per minute if blinkLights needed.
        OSTimeDlyHMSM(
            0,
            0,
            0,
            650, //650 milliseconds corresponds to 92ish times per minute 
            OS_OPT_TIME_HMSM_STRICT,
             &err
        );
    }
}
