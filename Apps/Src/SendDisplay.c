#include "SendDisplay.h"
#include "Minions.h"
#include "MotorController.h"
#include <math.h>

void Task_SendDisplay(void *p_arg) {
    OS_ERR err;

    while (1) {
        if(Display_SendNext() == ERROR){
            FaultBitmap |= FAULT_DISPLAY;
            
            OSSemPost(&FaultState_Sem4, OS_OPT_POST_1, &err);
            EnterFaultState();
        }
        
        OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_NON_STRICT, &err); // Update screen at roughly 10 fps
        assertOSError(OS_DISPLAY_LOC, err);
    }
}
