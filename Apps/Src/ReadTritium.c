/* Copyright (c) 2021 UT Longhorn Racing Solar */

#include "ReadTritium.h"
#include "CAN_Queue.h"
#include "CANbus.h"
#include <string.h>

void Task_ReadTritium(void *p_arg)
{

	OS_ERR err;

	CANDATA_t dataBuf;
	while (1)
	{
		CANbuff buf;
		ErrorStatus status = MotorController_Read(&buf);

		if (status == SUCCESS)
		{
			dataBuf.data = ((uint64_t)buf.firstNum << 32) | ((uint64_t)buf.secondNum);
			dataBuf.ID = buf.id;
			dataBuf.idxEn = false;

			__unused
				ErrorStatus error = CAN_Queue_Post(dataBuf);

			// TODO: handle error
		}
		OSTimeDlyHMSM(0, 0, 0, 10, OS_OPT_TIME_HMSM_NON_STRICT, &err);
		assertOSError(OS_READ_TRITIUM_LOC, err);
	}
}