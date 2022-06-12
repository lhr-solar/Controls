/* Copyright (c) 2021 UT Longhorn Racing Solar */

#include "ReadTritium.h"
#include "CAN_Queue.h"
#include "CANbus.h"
#include <string.h>

void Task_ReadTritium(void *p_arg)
{

	OS_ERR err;

	CANMSG_t msg;

	while (1)
	{
		CANbuff buf;
		ErrorStatus status = MotorController_Read(&buf);

		if (status == SUCCESS)
		{

			msg.id = buf.id;
			msg.payload.data.d = ((uint64_t)buf.firstNum << 32) | ((uint64_t)buf.secondNum);

			__unused
				ErrorStatus error = CAN_Queue_Post(msg);

			// TODO: handle error
		}
		OSTimeDlyHMSM(0, 0, 0, 10, OS_OPT_TIME_HMSM_NON_STRICT, &err);
		assertOSError(OS_READ_TRITIUM_LOC, err);
	}
}