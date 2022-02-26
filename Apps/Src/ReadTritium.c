/* Copyright (c) 2021 UT Longhorn Racing Solar */

#include "ReadTritium.h"

void Task_ReadTritium(void* p_arg) {

	OS_ERR err;
	
	car_state_t *car_state = (car_state_t *) p_arg;

	MotorController_Init();

	while (1) {
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