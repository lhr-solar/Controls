#include "Tasks.h"
#include "Minions.h"

/**
* @brief This thread controls the blinking of the hazard and indicator lights at the regulation specified frequency. 
**/

void Task_BlinkLight(void* p_arg){
    OS_ERR err;
    while(1){
        //lock thread to run at 90ish times per minute if blinkLights needed.
        OSTimeDlyHMSM(
            0,
            0,
            0,
            650, //650 milliseconds corresponds to 92ish times per minute 
            OS_OPT_TIME_HMSM_STRICT,
            &err
        );
        assertOSError(OS_BLINK_LIGHTS_LOC,err);
    }
    
}