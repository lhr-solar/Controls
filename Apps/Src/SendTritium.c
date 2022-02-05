#include "SendTritium.h"
#include "CarState.h"
#include <math.h>


// Convert a float velocity to a desired RPM
static float velocity_to_rpm(float velocity) {
    float velocity_mpm = velocity * 60.0f; // velocity in meters per minute
    const float circumfrence = WHEEL_DIAMETER * M_PI;
    float wheel_rpm = velocity_mpm / circumfrence;
    return wheel_rpm;
}


void Task_SendTritium(void *p_arg) {

    car_state_t *car_state = (car_state_t *) p_arg;

    OS_ERR err;
    CPU_TS ts;

    MotorController_Init();

    while (1) {
        OSTaskSemPend(0, OS_OPT_PEND_NON_BLOCKING, &ts, &err);
        if (err == OS_ERR_NONE) {
            // A signal was received, so the task should wait until signaled again
            OSTaskSemPend(0, OS_OPT_PEND_BLOCKING, &ts, &err);
            assertOSError(car_state, OS_SEND_TRITIUM_LOC, err);
        } else if (err != OS_ERR_PEND_WOULD_BLOCK) {
            assertOSError(car_state, OS_SEND_TRITIUM_LOC, err);
        }

        // Send the drive command to the motor controller
        MotorController_Drive(velocity_to_rpm(car_state->CruiseControlVelocity), car_state->DesiredMotorCurrent);

        // Delay for 100 ms
        OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &err);
        if (err != OS_ERR_NONE) {
            assertOSError(car_state, OS_SEND_TRITIUM_LOC, err);
        }
    }
}