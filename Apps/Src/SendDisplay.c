#include "SendDisplay.h"
#include "MotorController.h"
#include <math.h>

void Task_SendDisplay(void *p_arg) {
    OS_ERR err;

    Display_SetMainView(); // Make sure we're in the main view first
    Display_CruiseEnable(OFF);

    while (1) {
        float vel_mps = MotorController_ReadVelocity();
        float vel_mph = vel_mps * (2.23694);
        Display_SetVelocity(vel_mph);
        Display_SetSBPV(SupplementalVoltage);
        Display_SetChargeState(StateOfCharge);
        Display_SetRegenEnabled(RegenEnable);

        OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_NON_STRICT, &err); // Update screen at roughly 10 fps
    }
}