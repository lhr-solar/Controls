/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "BlinkerLight.h"

/**
* @brief   Toggle proper indicator lights
* @param   None
* @return  None
*/ 
void Task_BlinkLight(void *p_arg){
    car_state_t *carStates = ((car_state_t*)p_arg);
    
    OS_ERR err;
    CPU_TS ts;

    while(true){
        if(carStates->BlinkerStates.HZD){
            //Both lights blink at the same rate
            Lights_Set(LEFT_BLINK, !Lights_Read(LEFT_BLINK));
            Lights_Set(RIGHT_BLINK, !Lights_Read(LEFT_BLINK));

        }else if(carStates->BlinkerStates.LT){
            // Only the left light blinks
            Lights_Set(LEFT_BLINK, !Lights_Read(LEFT_BLINK));
            Lights_Set(RIGHT_BLINK, 0);

        }else if(carStates->BlinkerStates.RT){
            // Only the right light blinks
            Lights_Set(RIGHT_BLINK, !Lights_Read(RIGHT_BLINK));
            Lights_Set(LEFT_BLINK, 0);
        }

        // Wait 0.64 seconds
        OSTimeDlyHMSM (0, 0, 0, 640, OS_OPT_TIME_HMSM_STRICT, &err);
    }
}

