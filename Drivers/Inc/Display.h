/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file Display.h
 * @brief Function prototypes for the display driver.
 * 
 * This contains function prototypes relevant to sending/receiving messages
 * to/from our Nextion HMI.
 * 
 * @defgroup Display
 * @addtogroup Display
 * @{
 */

#ifndef __DISPLAY_H
#define __DISPLAY_H

#include "common.h"	// common headers
#include "Tasks.h"	// for os and fault error locs

#define MAX_ARGS 2	// maximum # of arguments in a command packet

/**
 * Error types
 */
typedef enum{ // Currently only ERR_NONE and ERR_PARSE are used
	DISPLAY_ERR_NONE,
	DISPLAY_ERR_PARSE,	    // Error parsing command struct passed to Display_Send
	DISPLAY_ERR_INV_INSTR,	// Invalid instruction passed to nextion (0x00)
	DISPLAY_ERR_INV_COMP,	// Invalid component id passed to nextion (0x02)
	DISPLAY_ERR_INV_PGID,	// Invalid page id passed to nextion	(0x03)
	DISPLAY_ERR_INV_VAR,	// Invalid variable name passed to nextion	(0x1A)
	DISPLAY_ERR_INV_VAROP,	// Invalid variable operation passed to nextion	(0x1B)
	DISPLAY_ERR_ASSIGN,	    // Assignment failure nextion	(0x1C)
	DISPLAY_ERR_PARAMS,	    // Invalid number of parameters passed to nextion	(0x1E)
	DISPLAY_ERR_MAX_ARGS,   // Command arg list exceeded MAX_ARGS elements
	DISPLAY_ERR_OTHER       // Other nextion display error
} DisplayError_t;


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
	Arg_e argTypes[MAX_ARGS];
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
 * @returns DisplayError_t
 */
DisplayError_t Display_Error();

/**
 * @brief Overwrites any processing commands and triggers the evacuation screen
 * @param SOC_percent the state of charge of the battery in percent
 * @param supp_mv the voltage of the battery in millivolts
 * @returns DisplayError_t
*/
DisplayError_t Display_Evac(uint8_t SOC_percent, uint32_t supp_mv);

#endif


/* @} */
