#include "SendTritium.h"

void Task_SendTritium(void *p_arg) {

    (void) p_arg;

    OS_ERR err;
    CPU_TS ts;

    // Wait until the motor has been activated
    OSSemPend(&SendTritium_Sem4, 0, OS_OPT_PEND_BLOCKING, &ts, &err);
    // TODO: error handling

    MotorController_Init();

    while (1) {
        // TODO read these from the globals list
        float requested_rpm = 0.0f;
        float requested_current_percent = 0.0f;

        // Send the drive command to the motor controller
        MotorController_Drive(requested_rpm, requested_current_percent);

        // Delay for 100 ms
        OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &err);
    }
}