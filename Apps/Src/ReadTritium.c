/* Copyright (c) 2021 UT Longhorn Racing Solar */

#include "ReadTritium.h"

void Task_ReadTritium(void* p_arg) {

	OS_ERR err;

	while (1) {
		CANbuff buf;
		ErrorStatus status = MotorController_Read(&buf);

		if(status == SUCCESS) {

			OSQPost(&CANBus_MsgQ, (void *) &buf, sizeof(buf), OS_OPT_POST_FIFO, &err);
		}
		OSTimeDlyHMSM(0, 0, 0, 10, OS_OPT_TIME_HMSM_NON_STRICT, &err);
		assertOSError(OS_READ_TRITIUM_LOC, err);
	}
}