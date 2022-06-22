

#include "SendDisplay.h"
#include "MotorController.h"

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
    // car_state_t *car = (car_state_t *) p_arg;
    OS_ERR err;

    Display_SetMainView(); // Make sure we're in the main view first
    
    while (1) {
        Display_SetVelocity(MotorController_ReadVelocity());
        Display_SetSBPV(SupplementalVoltage);

        OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_NON_STRICT, &err); // Update screen at roughly 10 fps
    }
}