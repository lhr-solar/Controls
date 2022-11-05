/* Copyright (c) 2021 UT Longhorn Racing Solar */

#include "ReadTritium.h"
#include "CAN_Queue.h"
#include "CANbus.h"
#include <string.h>

/* OBJECTIVES:
Objective 1:
- Receive motor status message from MotorController (18.4.2)
- interpret error status
	- if error
		- assertOSError
- determine information important to telementry
	- SendCarCAN
- determine information important for storage
	- acquire mutex on Storage Array
	- Store information in Storage Array (based on index) 
	- release mutex on Storage Array

Objective 2:
- create function able to read data from Storage Array
	- pend on Storage Array mutex
	- acquire Storage Array mutex
	- read information of array index 
	- release Storage Array mutex
*/

void Task_ReadTritium(void *p_arg)
{

	OS_ERR err;

	CANMSG_t msg;

	CANId_t ID;
	CANDATA_t data;
	CAN_blocking_t blocking;

  
	while (1)
	{
		ErrorStatus status = CANbus_Read(CANDATA_t* data, CAN_blocking_t blocking, CAN_t MOTORCAN);

/*		CANbuff buf;
		ErrorStatus status = MotorController_Read(&buf);

		if (status == SUCCESS)
		{

			msg.id = buf.id;
			msg.payload.data.d = ((uint64_t)buf.firstNum << 32) | ((uint64_t)buf.secondNum);

			__unused
				ErrorStatus error = CAN_Queue_Post(msg);

			// TODO: handle error
		}
*/
		OSTimeDlyHMSM(0, 0, 0, 10, OS_OPT_TIME_HMSM_NON_STRICT, &err);
		assertOSError(OS_READ_TRITIUM_LOC, err);
	}
}