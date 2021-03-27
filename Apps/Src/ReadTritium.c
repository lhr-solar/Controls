/* Copyright (c) 2021 UT Longhorn Racing Solar */

#include "ReadTritium.h"

void Task_ReadTritium(void* p_arg) {

	OS_ERR err;

	CANbus_Init();

	while (1) {

		uint8_t message[8];
		ErrorStatus status = CANbus_Read(message);
		printf("got data\n");

		if(status == SUCCESS) {

			// Is using strlen here correct?
			OSQPost(&CANBus_MsgQ, (void *)message, 8, OS_OPT_POST_FIFO, &err);
			
			if (err != OS_ERR_NONE) {
				// TODO: error handling
			}
		}
		OSTimeDlyHMSM(0, 0, 0, 1000, OS_OPT_TIME_HMSM_NON_STRICT, &err);
	}
}