/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "RTOSPedals.h"

/**
* @brief   Initialize both pedals to 0%
* @param   None
* @return  None
*/ 
static void Pedals_Values_Init(void){
    accelerator_percentage = 0;
    brake_percentage = 0;
}

/**
* @brief   Read both pedal percentage values and set the global variables
* @param   None
* @return  None
*/ 
void Task_ReadPedals(void *p_arg){
    Pedals_Values_Init();
    
    OS_ERR err;

    while(1){
        accelerator_percentage = Pedals_Read(ACCELERATOR);
        brake_percentage = Pedals_Read(BRAKE);

        // Post to semaphores (velocityChange, lightsChange)
        OSSemPost (&VelocityChange_Sem4, OS_OPT_POST_ALL, &err);
        OSSemPost (&LightsChange_Sem4, OS_OPT_POST_ALL, &err);
    }
}



