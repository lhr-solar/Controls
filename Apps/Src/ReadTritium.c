/* Copyright (c) 2021 UT Longhorn Racing Solar */

#include "ReadTritium.h"
#include "CAN_Queue.h"
#include "CANbus.h"
#include <string.h>

void Task_ReadTritium(void* p_arg) {

	OS_ERR err;
	
	car_state_t *car_state = (car_state_t *) p_arg;

	CANMSG_t msg;

	MotorController_Init(1.0f); // Let motor controller use 100% of bus current

	while (1) {
		CANbuff buf;
		ErrorStatus status = MotorController_Read(&buf);

		if(status == SUCCESS) {

			msg.id = buf.id;
			msg.payload.data.d = ((uint64_t) buf.firstNum << 32) | ((uint64_t) buf.secondNum);

			ErrorStatus error = CAN_Queue_Post(msg);
			
			if (error != SUCCESS) {
        	    car_state->ErrorCode.ReadTritiumErr = ON;
			}
		}
		OSTimeDlyHMSM(0, 0, 0, 10, OS_OPT_TIME_HMSM_NON_STRICT, &err);
		if (err != OS_ERR_NONE) {
			car_state->ErrorCode.ReadTritiumErr = ON;
		}
	}
}