#include "SendTritium.h"
#include "CarState.h"
#include <math.h>


static float velocity_to_rpm(float velocity) {
    float velocity_mpm = velocity * 60.0f; // velocity in meters per minute
    float circumfrence = WHEEL_DIAMETER * M_PI;
    float wheel_rpm = velocity_mpm / circumfrence;
    return wheel_rpm * GEAR_REDUCTION;
}


void Task_SendTritium(void *p_arg) {

    car_state_t *car_state = (car_state_t *) p_arg;

    OS_ERR err;

    MotorController_Init();

    while (1) {

        // Send the drive command to the motor controller
        MotorController_Drive(velocity_to_rpm(car_state->CruiseControlVelocity), car_state->DesiredMotorCurrent);

        // Delay for 100 ms
        OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &err);
    }
}