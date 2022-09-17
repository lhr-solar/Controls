/** Copyright (c) 2022 UT Longhorn Racing Solar
 * @file Display.c
 * @brief Function prototypes for the display driver.
 * 
 * This contains function prototypes relevant to sending/receiving messages
 * to/from our Nextion HMI.
 * 
 * @author Ishan Deshpande (IshDeshpa)
 * @author Roie Gal (Cam0Cow)
*/

#ifndef __DISPLAY_H
#define __DISPLAY_H

#include "common.h"	// common headers
#include "config.h"	// for ErrorStatus
#include "Tasks.h"	// for os and fault error locs

#define MAX_ARGS 2	// maximum # of arguments in a command packet

/**
 * All three pages on the HMI 
 */
typedef enum{
	STARTUP=0,
	INFO,
	FAULT
} Page_t;

/**
 * For display elements with three states
 */
typedef enum{
	DISABLED=0,
	ENABLED,
	ACTIVE
} TriState_t;

/**
 * Packages relevant display command data for use in a fifo
 */
typedef struct{
	char* compOrCmd;
	char* attr;
	char* op;
	uint8_t numArgs;
	bool argTypes[MAX_ARGS];	// TRUE for integers, FALSE for strings
	union{
		char* str;
		uint32_t num;
	} args[MAX_ARGS];
} Display_Cmd_t;

/**
 * @brief Sends a display message.
 * @returns ErrorStatus: ERROR or SUCCESS
 */
ErrorStatus Display_Send(Display_Cmd_t cmd);

/**
 * @brief Initializes the display
 * @returns ErrorStatus: ERROR or SUCCESS
 */
ErrorStatus Display_Init();

/**
 * @brief Resets (reboots) the display
 * @returns ErrorStatus: ERROR or SUCCESS
 */
ErrorStatus Display_Reset();

/**
 * @brief Overwrites any processing commands and triggers the display fault screen
 * @param osErrCode the os error location (will be displayed in hex)
 * @param faultCode the generic fault code (will be displayed in hex)
 * @returns ErrorStatus: ERROR or SUCCESS
 */
ErrorStatus Display_Fault(os_error_loc_t osErrCode, fault_bitmap_t faultCode);

#endif