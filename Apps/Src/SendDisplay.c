#include "SendDisplay.h"
#include "Minions.h"
#include "MotorController.h"
#include <math.h>

void Task_SendDisplay(void *p_arg) {
    OS_ERR err;

    Display_SetMainView(); // Make sure we're in the main view first
    Display_CruiseEnable(OFF);
    Display_SetGear(OFF, OFF);
    Display_SetLight(CTRL_FAULT,OFF);

    for(int i=0; i<6; i++)
        Display_SetLight(i, OFF);

    while (1) {
        float vel_mps = MotorController_ReadVelocity();
        Display_SetVelocity(vel_mps);
        Display_SetSBPV(SupplementalVoltage);
        Display_SetChargeState(StateOfCharge);
        Display_SetRegenEnabled(RegenEnable);
        Display_SetLight(A_CNCTR, Contactors_Get(ARRAY_CONTACTOR));
        Display_SetLight(M_CNCTR, Contactors_Get(MOTOR_CONTACTOR));
        Display_SetGear(Switches_Read(FOR_SW), Switches_Read(REV_SW));
        Display_SetLight(Headlight_ON, Switches_Read(HEADLIGHT_SW));

        OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_NON_STRICT, &err); // Update screen at roughly 10 fps
    }
}
