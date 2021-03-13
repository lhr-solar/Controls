/* Copyright (c) 2021 UT Longhorn Racing Solar */

#include "ReadTritium.h"

void Task_ReadTritium(void* p_arg) {

	OS_ERR err;

	CANbus_Init();

	while (1) {

		uint8_t *message;
		ErrorStatus status = CANbus_Read(message);

		if(status == SUCCESS) {

			OSQPost(&CANBus_MsgQ, (void *)message, strlen((char *) message), OS_OPT_POST_FIFO, &err);
			
			if (err != OS_ERR_NONE) {
				// TODO: error handling
			}
		}
	}
}