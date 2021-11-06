/* Copyright (c) 2021 UT Longhorn Racing Solar */

#include "ReadTritium.h"

void Task_ReadTritium(void* p_arg) {

	OS_ERR err;
	CPU_TS ts;
	
	car_state_t *car_state = (car_state_t *) p_arg;

	MotorController_Init();

	while (1) {
		OSTaskSemPend(0, OS_OPT_PEND_NON_BLOCKING, &ts, &err);
        if (err == OS_ERR_NONE) {
            // A signal was received, so the task should wait until signaled again
            OSTaskSemPend(0, OS_OPT_PEND_BLOCKING, &ts, &err);
            // TODO: error handling
        } else if (err != OS_ERR_PEND_WOULD_BLOCK) {
            // TODO: error handling
        }

		CANbuff buf;
		ErrorStatus status = MotorController_Read(&buf);

		if(status == SUCCESS) {

			OSQPost(&CANBus_MsgQ, (void *) &buf, sizeof(buf), OS_OPT_POST_FIFO, &err);
			
			if (err != OS_ERR_NONE) {
        	    car_state->ErrorCode.ReadTritiumErr = ON;
			}
		}
		OSTimeDlyHMSM(0, 0, 0, 10, OS_OPT_TIME_HMSM_NON_STRICT, &err);
		if (err != OS_ERR_NONE) {
			car_state->ErrorCode.ReadTritiumErr = ON;
		}
	}
}