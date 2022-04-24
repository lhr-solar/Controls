#include "Tasks.h"
#include "Minions.h"
#include "os.h"
#include "common.h"

/**
* @brief This thread controls the blinking of the hazard and indicator lights at the regulation specified frequency. 
**/

void Task_BlinkLight(void* p_arg){
    OS_ERR err;
    while(1){
        Lights_Set(LEFT_BLINK, (State) (Lights_Toggle_Read(LEFT_BLINK) ^ Lights_Read(LEFT_BLINK)));
        Lights_Set(RIGHT_BLINK, (State) (Lights_Toggle_Read(RIGHT_BLINK) ^ Lights_Read(RIGHT_BLINK)));
        
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