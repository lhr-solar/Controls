/* Copyright (c) 2020 UT Longhorn Racing Solar */

#include "RTOSPedals.h"

//Accelerator and Brake Percentage
int8_t accelerator_percentage;
int8_t brake_percentage;

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

/**
* @brief   Getter function for local variable brake_percentage
* @param   None
* @return  brake_percentage
*/ 
uint8_t RTOSPedals_GetBrakePercentage(void){
    return brake_percentage;
}

/**
* @brief   Getter function for local variable accelerator_percentage
* @param   None
* @return  accelerator_percentage
*/ 
uint8_t RTOSPedals_GetAcceleratorPercentage(void){
    return accelerator_percentage;
}



