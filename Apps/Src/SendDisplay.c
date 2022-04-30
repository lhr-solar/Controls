

#include "SendDisplay.h"

#define MAX_DISPLAYABLE_ERRORS 6

/*
// Strings for motor errors
static const char *MOTOR_ERROR_STRINGS[] = {
    "MOTOR TEMP ERR",
    "VELOCITY LIMITED", // Not really an error?
    "SLIP SPEED ERR",
    "OVERSPEED ERR"     // Not implemented, unecessary
};

// Strings for software module errors
static const char *ERROR_STRINGS[] = {
    "ARRAY ERR",
    "READ CAN ERR",
    "READ TRITIUM ERR",
    "SEND CAN ERR",
    "SEND TRITIUM ERR",
    "UPDATE VELOCITY ERR",
    "READ PEDAL ERR",
    "BLINK LIGHTS ERR",
    "MOTOR CONNECTION ERR"
};
*/

void Task_SendDisplay(void *p_arg) {
    car_state_t *car = (car_state_t *) p_arg;
    OS_ERR err;

    Display_Init();

    Display_SetPrechargeView();

    while (1) {

        /*
        Error display commented out for now
        const char *errors[MAX_DISPLAYABLE_ERRORS]; // Up to 6 errors possible
        size_t errorCount = 0;

        union error_array_t error_arr;
        error_arr.err = car->ErrorCode;
        State *errorStates = error_arr.arr;

        // check for the motor errors we care about
        if (car->MotorErrorCode.motorTempErr) {
            errors[errorCount++] = MOTOR_ERROR_STRINGS[0];
        }

        if (car->MotorErrorCode.slipSpeedErr) {
            errors[errorCount++] = MOTOR_ERROR_STRINGS[2];
        }

        // Note that we only ever display the first six errors
        // These errors will always be displayed after motor errors
        for (int i=0; i < SYSTEM_ERR_COUNT && errorCount < MAX_DISPLAYABLE_ERRORS; i++) {
            if (errorStates[i] == ON) {
                errors[errorCount++] = ERROR_STRINGS[i];
            }
        }

        */

        // If the contactors are not yet enabled, we're probably still in precharge
        // Otherwise, changed to the main view
        if (Contactors_Get(ARRAY_CONTACTOR) == ON) {
            Display_SetMainView(); // Make sure we're in the main view first
            Display_SetVelocity(car->CurrentVelocity);
            Display_CruiseEnable(car->CruiseControlEnable);
            Display_CruiseSet(car->CRSet == CRUISE ? ON : OFF);

            /*
            // update error display
            int i = 0;
            for (; i < errorCount; i++) {
                Display_SetError(i, (char *) errors[i]);
            }
            for (; i<6; i++) {
                Display_SetError(i, ""); // Clear unsused error slots
            }

            // If we have no errors, change color to green and set text to "No Errors"
            if (errorCount == 0) {
                Display_NoErrors();
            }
            */
        }

        OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_NON_STRICT, &err); // Update screen at roughly 10 fps
    }
}