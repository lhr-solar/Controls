

#include "SendDisplay.h"

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

void Task_SendDisplay(void *p_arg) {
    car_state_t *car = (car_state_t *) p_arg;
    OS_ERR err;

    // Get velocity in deci-meters per second
    int currentVelocity = floorf(car->CurrentVelocity * 10.0f);
    

    char *errors[6]; // Up to 6 errors possible
    size_t errorCount = 0;

    // Access the error codes as an array so we can loop through them
    State *errorStates = ((State *) &(car->ErrorCode));
    size_t numErrors = sizeof(error_code_t) / sizeof(State);

    // Note that we only ever display the first six errors
    for (int i=0; i < numErrors && errorCount < 6; i++) {
        if (errorStates[i] == ON) {
            errors[errorCount++] = ERROR_STRINGS[i];
        }
    }


    OSTimeDlyHMSM(0, 0, 0, 33, OS_OPT_TIME_HMSM_NON_STRICT, &err); // Update screen at roughly 30 fps
}