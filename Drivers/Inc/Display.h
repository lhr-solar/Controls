/** 
 * @copyright Copyright (c) 2022 UT Longhorn Racing Solar
 * @file Display.c
 * @brief Function prototypes for the display driver.
 * 
 * This contains function prototypes relevant to sending/receiving messages
 * to/from our Nextion HMI. Call assertDisplayError after calling any of the
 * functions in this application.
 * 
 * @author Ishan Deshpande (IshDeshpa)
 * @author Roie Gal (Cam0Cow)
 * @author Nathaniel Delgado (NathanielDelgado)
*/

#ifndef __DISPLAY_H
#define __DISPLAY_H

#include "common.h"	// common headers
#include "Tasks.h"	// for os and fault error locs

#define MAX_ARGS 2	// maximum # of arguments in a command packet

/**
 * Error types
 */
typedef enum{
	DISPLAY_ERR_NONE      = 0,
	DISPLAY_ERR_PARSE     =-1,	// Error parsing command struct passed to Display_Send
	DISPLAY_ERR_INV_INSTR =-2,	// Invalid instruction passed to nextion (0x00)
	DISPLAY_ERR_INV_COMP  =-3,	// Invalid component id passed to nextion (0x02)
	DISPLAY_ERR_INV_PGID  =-4,	// Invalid page id passed to nextion	(0x03)
	DISPLAY_ERR_INV_VAR   =-5,	// Invalid variable name passed to nextion	(0x1A)
	DISPLAY_ERR_INV_VAROP =-6,	// Invalid variable operation passed to nextion	(0x1B)
	DISPLAY_ERR_ASSIGN    =-7,	// Assignment failure nextion	(0x1C)
	DISPLAY_ERR_PARAMS    =-8,	// Invalid number of parameters passed to nextion	(0x1E)
	DISPLAY_ERR_MAX_ARGS  =-9,  // Command arg list exceeded MAX_ARGS elements
	DISPLAY_ERR_OTHER     =-10  // Other nextion display error
} DisplayError_t;

/**
 * @brief Error handler for any display errors. Call this after any display driver function.
 * Restarts the display if there is an error 
 * or displays fault if it has reached the threshold for restart attempts
 * @param err the display error code to check
 */
void assertDisplayError(DisplayError_t err);

/**
 * All three pages on the HMI 
 */
typedef enum{
	STARTUP	=0,
	INFO,
	FAULT
} Page_t;

/**
 * Argument types
 */
typedef enum{
	STR_ARG,
	INT_ARG
}	Arg_e;

/**
 * Packages relevant display command data
 */
typedef struct{
	char* compOrCmd;
	char* attr;
	char* op;
	uint8_t numArgs;
	Arg_e argTypes[MAX_ARGS];	// TRUE for integers, FALSE for strings
	union{
		char* str;
		uint32_t num;
	} args[MAX_ARGS];
} DisplayCmd_t;

/**
 * @brief Sends a display message.
 * @returns DisplayError_t
 */
DisplayError_t Display_Send(DisplayCmd_t cmd);

/**
 * @brief Initializes the display
 * @returns DisplayError_t
 */
DisplayError_t Display_Init(void);

/**
 * @brief Resets (reboots) the display
 * @returns DisplayError_t
 */
DisplayError_t Display_Reset(void);

/**
 * @brief Overwrites any processing commands and triggers the display fault screen
 * @param osErrCode the os error location (will be displayed in hex)
 * @param faultCode the generic fault code (will be displayed in hex)
 * @returns DisplayError_t
 */
DisplayError_t Display_Fault(os_error_loc_t osErrCode, fault_bitmap_t faultCode);

/**
 * @brief Overwrites any processing commands and triggers the evacuation screen
 * @param SOC_percent the state of charge of the battery in percent
 * @param supp_mv the voltage of the battery in millivolts
 * @returns DisplayError_t
*/
DisplayError_t Display_Evac(uint8_t SOC_percent, uint32_t supp_mv);

#endif