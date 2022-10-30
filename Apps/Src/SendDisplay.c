#include "SendDisplay.h"
#include "MotorController.h"
#include <math.h>
#include "Minions.h"

typedef enum {
     HZD_SW=0, 
     HEADLIGHT_SW,
     LEFT_SW,
     RIGHT_SW,
} switches_t;


typedef enum {  
    A_CNCTR = 0,
    M_CNCTR,
    CTRL_FAULT,
    LEFT_BLINK,
    RIGHT_BLINK,
    Headlight_ON,
    BrakeLight,
    RSVD_LED
} light_t;

void Task_SendDisplay(void *p_arg) {
    OS_ERR err;
    Minion_Error_t mErr; 

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
        Display_SetGear(Minion_Read_Input(FOR_SW, &mErr), Minion_Read_Input(REV_SW, &mErr));
        Display_SetLight(Headlight_ON, Minion_Read_Input(HEADLIGHT_SW, &mErr));

        OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_NON_STRICT, &err); // Update screen at roughly 10 fps
    }
}
