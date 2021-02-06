/* Copyright (c) 2021 UT Longhorn Racing Solar */

#include "ReadTritium.h"

void Task_ReadTritium(void* p_arg){

	OS_ERR err;
	CPU_TS ts;

	OSSEMPend(&READTritium_Sem4, 0, OS_OPT_PEND_BLOCKING, &ts, &err);

	CANbus_Init();

	uint8_t *message;
	ErrorStatus status = CANbus_Read(message);
}