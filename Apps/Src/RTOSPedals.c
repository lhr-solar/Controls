/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "RTOSPedals.h"

/**
* @brief   Read both pedal percentage values and set the global variables
* @param   None
* @return  None
*/ 
void Task_ReadPedals(void *p_arg){
    
    car_state_t *carStates = ((car_state_t*)p_arg);
    OS_ERR err;

    uint8_t PreviousAccel = carStates->AccelPedalPercent;
    uint8_t PreviousBrake = carStates->BrakePedalPercent;

    while(1){
        carStates->AccelPedalPercent = Pedals_Read(ACCELERATOR);
        carStates->BrakePedalPercent = Pedals_Read(BRAKE);
        
        // The velocity and lights threads can run without blocking
        // semaphores, the delay added an the end of this loop will allow
        // them to run

        // Post to semaphores if there is a change in newly read accel or brake compared to prev values
        if(carStates->AccelPedalPercent != PreviousAccel){
            OSSemPost (&VelocityChange_Sem4, OS_OPT_POST_ALL, &err);
        }
        if( carStates->BrakePedalPercent != PreviousBrake){
            OSSemPost (&LightsChange_Sem4, OS_OPT_POST_ALL, &err);
        }

        PreviousAccel = carStates->AccelPedalPercent;
        PreviousBrake = carStates->BrakePedalPercent;
    
        // Delay for 0.01 sec
        OSTimeDlyHMSM (0, 0, 0, 10, OS_OPT_TIME_HMSM_STRICT, &err);
    }
}



