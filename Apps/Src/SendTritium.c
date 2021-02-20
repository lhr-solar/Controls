#include "SendTritium.h"

void Task_SendTritium(void *p_arg) {

    (void) p_arg;

    OS_ERR err;

    MotorController_Init();

    while (1) {
        float requested_rpm = RequestedMotorRPM;
        float requested_current_percent = RequestedMotorCurrentPercent;

        // Send the drive command to the motor controller
        MotorController_Drive(requested_rpm, requested_current_percent);

        // Delay for 100 ms
        OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &err);
    }
}