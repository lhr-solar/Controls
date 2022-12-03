#include "Tasks.h"
#include "CANbus.h"
#include "CAN_Queue.h"
#include "common.h"

/**
 * @file SendCarCAN.c
 * @brief Implements the SendCarCAN Task
 * 
 * Resends the feedback from the motor controller
 * 
 */

/**
 * @brief Sends the feedback from the motor controller to be read by telemetry and 
 * sends pedal, switch, light, and contactor information to be read by telemetry
 * 
 * @param p_arg 
 */
void Task_SendCarCAN(void *p_arg){
    CANMSG_t motorMsg;
    OS_ERR err;

    while (1) {
        // Send motor msg
        CAN_Queue_Pend(&motorMsg);
        CANbus_Send(motorMsg.id, motorMsg.payload, CAN_BLOCKING);

        // Delay of few milliseconds (100)
        OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &err);
        if (err != OS_ERR_NONE){
            assertOSError(OS_UPDATE_VEL_LOC, err);
        }
    }
}
