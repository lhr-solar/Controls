/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "RTOSPedals.h"


/**
* @brief   Read both pedal percentage values and set the global variables
* @param   None
* @return  None
*/ 
void Task_ReadPedals(void *p_arg){
    OS_ERR err;

    while(1){
        pedals.ACC_PERCENT = Pedals_Read(ACCELERATOR);
        pedals.BRAKE_PERCENT = Pedals_Read(BRAKE);
        // The velocity and lights threads can run without blocking
        // semaphores, the delay added an the end of this loop will allow
        // them to run

        // Post to semaphores (velocityChange, lightsChange) 
        // OSSemPost (&VelocityChange_Sem4, OS_OPT_POST_ALL, &err);
        // OSSemPost (&LightsChange_Sem4, OS_OPT_POST_ALL, &err);

        // Delay for 0.01 sec
        OSTimeDlyHMSM (0, 0, 0, 10, OS_OPT_TIME_HMSM_STRICT, &err);
    }
}



