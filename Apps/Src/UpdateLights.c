#include "Lights.h"
#include "Tasks.h"
#define BRAKETHRESHOLD 2

/**
 * @brief This task waits on the Update Lights Semaphore, then reads the switches that pertain to the lights from the global carstate
 * (UpdateSwitches will have already updated this). Using those values it uses the lights driver to set the lights to the pertinent values.
 * 
 * @param p_arg 
 */
void Task_UpdateLights(void* p_arg){
    car_state_t* GlobalCarState = (car_state_t*) p_arg;
    switch_states_t* GlobalSwitchStates = &(GlobalCarState->SwitchStates);
    //Note: the below code is based off the switch_states_t struct defined in the RTOS_ReadSwitches branch rather than the current one
    light_switches_t* GlobalLightSwitches = &(GlobalSwitchStates->lightSwitches);
    OS_ERR err;
    CPU_TS ts;
    while(1){
        OSSemPend(
            &LightsChange_Sem4,
            0,
            OS_OPT_PEND_BLOCKING,
            &ts,
            &err
        );
        if(GlobalLightSwitches->HEADLIGHT_SW == ON){
            Lights_Set(Headlight_ON,ON);
        } else {
            Lights_Set(Headlight_ON,OFF);
        }

        if(GlobalLightSwitches->HZD_SW == ON){
            //TODO: Signal BlinkLights Sem4
        } else if (GlobalLightSwitches->LEFT_SW==ON){
            //TODO: Signal BlinkLights Sem4
        } else if (GlobalLightSwitches->RIGHT_SW==ON){
            //TODO: Signal BlinkLights Sem4
        } else {
            Lights_Set(LEFT_BLINK,OFF);
            Lights_Set(RIGHT_BLINK,OFF);
        }

        if(GlobalCarState->BrakePedalPercent>=BRAKETHRESHOLD){
            Lights_Set(BrakeLight,ON);
        } else {
            Lights_Set(BrakeLight,OFF);
        }

        if(GlobalCarState->ShouldArrayBeActivated==ON){
            Lights_Set(A_CNCTR,ON);
        } else {
            Lights_Set(A_CNCTR,OFF);
        }

        if(GlobalCarState->ShouldMotorBeActivated==ON){
            Lights_Set(M_CNCTR,ON);
        } else {
            Lights_Set(M_CNCTR,OFF);
        }

    }
}