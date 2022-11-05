#include "Minions.h"
#include "os.h"
#include "Contactors.h"
#include "Tasks.h"

void Task_Contactor_Ignition(void) {
    OS_ERR err;
    Minion_Error_t Merr;

    // Delay for precharge
    OSTimeDlyHMSM(0, 0, PRECHARGE_MOTOR_DELAY, 0, OS_OPT_TIME_HMSM_NON_STRICT, &err);
    assertOSError(OS_MINIONS_LOC, err);

    Contactors_Enable(MOTOR_CONTACTOR);
    // static bool lastCruiseEnPushed = true; //cruise control edge detector variables
    // static State cruiseEnablePushed = OFF;
    // Main loop
    while (1) {

        //UpdateSwitches();

        // array on/off
        if (Minion_Read_Input(IGN_1, &Merr) == true || Minion_Read_Input(IGN_2, &Merr) == true) {
            Contactors_Enable(ARRAY_CONTACTOR);
            Contactors_Enable(ARRAY_PRECHARGE);
        } else {
            Contactors_Disable(ARRAY_CONTACTOR);
            Contactors_Disable(ARRAY_PRECHARGE);
            //Lights_Set(A_CNCTR,OFF);
            //Display_SetLight(A_CNCTR,OFF);
        }
        assertOSError(OS_MINIONS_LOC, err);

        // motor on/off
        Contactors_Set(MOTOR_CONTACTOR, Minion_Read_Input(IGN_2, &Merr));
        assertOSError(OS_MINIONS_LOC, err);
        //Lights_Set(M_CNCTR,Switches_Read(IGN_2));

        //cruiseEnablePushed = Switches_Read(CRUZ_EN); //read cruise enable switch
        // if (!lastCruiseEnPushed && cruiseEnablePushed) { //Rising Edge detector for updateVelocity thread
        //     UpdateVel_ToggleCruise = true; //updateVelocity thread depends on this
        // }

        //lastCruiseEnPushed = cruiseEnablePushed;


        //OSTimeDlyHMSM(0, 0, 0, READ_SWITCH_PERIOD, OS_OPT_TIME_HMSM_NON_STRICT, &err);
        //assertOSError(OS_SWITCHES_LOC, err);
    }
}