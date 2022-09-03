/* Copyright (c) 2020 UT Longhorn Racing Solar */

/* Driver for the display board. This communicates
 * with the Nextion display over UART in order to show
 * critical information to the driver.
 */

#ifndef __DISPLAY_H
#define __DISPLAY_H

#include "common.h"
#include "config.h"
#include "Minions.h"

#define MAX_ARGS 2

typedef enum{
	STARTUP=0,
	INFO,
	FAULT
} Page_t;

typedef struct{
	char* compOrCmd;
	char* attr;
	char* op;
	uint8_t numArgs;
	bool argTypes[MAX_ARGS];	// TRUE for integers, FALSE for strings
	union{
		uint8_t num;
		char* str;
	} args[MAX_ARGS];
} Display_Cmd_t;

OS_SEM DisplayQ_Sem4;
OS_MUTEX DisplayQ_Mutex;

ErrorStatus Display_Init();

ErrorStatus Display_Reset();

ErrorStatus Display_SetSOC(uint8_t percent);

ErrorStatus Display_SetSBPV(uint16_t mv);

ErrorStatus Display_SetGear(uint8_t gear);

ErrorStatus Display_SetArray(bool state);

ErrorStatus Display_SetMotor(bool state);

ErrorStatus Display_SetRegenEnable(bool state);

ErrorStatus Display_SetCruiseEnable(bool state);

ErrorStatus Display_SetLeftBlink(bool state);

ErrorStatus Display_SetRightBlink(bool state);

ErrorStatus Display_Fault(os_error_loc_t osErrCode, fault_bitmap_t faultCode);

ErrorStatus Display_SendNext();

#endif