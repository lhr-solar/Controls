/* Copyright (c) 2021 UT Longhorn Racing Solar */

#include "ReadTritium.h"

void Task_ReadTritium(void* p_arg){

	OS_ERR err;
	CPU_TS ts;

	OSSEMPend(&READTritium_Sem4, 0, OS_OPT_PEND_BLOCKING, &ts, &err);

	CANbus_Init();

	uint8_t *message;
	ErrorStatus status = CANbus_Read(message);

	if(message){

		err = OSQPost(CANbus_MsgQ, (void *)message);
		
		switch(err){
			case OS_Q_FULL:
				break;
			case OS_ERR_EVENT_TYPE:
				break;
			case OS_ERR_PEVENT_NULL:
				break;
			case OS_ERR_POST_NULL_PTR:
				// This shouldn't happen since we check if message was null earlier
				break;
		}

	}
	// Not sure what to do if message returns null
}