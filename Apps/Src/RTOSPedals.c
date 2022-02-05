/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "RTOSPedals.h"

/**
* @brief   Read both pedal percentage values and set the global variables
* @param   None
* @return  None
*/ 
void Task_ReadPedals(void *p_arg){
    
    car_state_t *car_state = ((car_state_t*)p_arg);
    OS_ERR err;

    uint8_t PreviousAccel = car_state->AccelPedalPercent;
    uint8_t PreviousBrake = car_state->BrakePedalPercent;

    while(1){
        car_state->AccelPedalPercent = Pedals_Read(ACCELERATOR);
        car_state->BrakePedalPercent = Pedals_Read(BRAKE);
        
        // The velocity and lights threads can run without blocking
        // semaphores, the delay added an the end of this loop will allow
        // them to run

        // Post to semaphores if there is a change in newly read accel or brake compared to prev values
        if(car_state->AccelPedalPercent != PreviousAccel){
            OSSemPost (&VelocityChange_Sem4, OS_OPT_POST_ALL, &err);
            assertOSErr(car_state, UPDATE_VEL_ERR, M_NONE, &err);
        }
        if( car_state->BrakePedalPercent != PreviousBrake){
            OSSemPost (&LightsChange_Sem4, OS_OPT_POST_ALL, &err);
            assertOSErr(car_state, BLINK_LIGHTS_ERR, M_NONE, &err);
        }

        PreviousAccel = car_state->AccelPedalPercent;
        PreviousBrake = car_state->BrakePedalPercent;
    
        // Delay for 0.01 sec
        OSTimeDlyHMSM (0, 0, 0, 10, OS_OPT_TIME_HMSM_STRICT, &err);
        assertOSErr(car_state, READ_PEDAL_ERR, M_NONE, &err);
    }
}



