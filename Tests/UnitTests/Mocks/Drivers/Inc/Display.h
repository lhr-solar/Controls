#ifdef TEST_DISPLAY
#include_next "Display.h"
#else
#ifndef __DISPLAY_H
#define __DISPLAY_H

#include "common.h"	// common headers
#include "Tasks.h"
#include "fff.h"

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
	Arg_e argTypes[MAX_ARGS];	// TRUE for integers, FALSE for strings
	union{
		char* str;
		uint32_t num;
	} args[MAX_ARGS];
} DisplayCmd_t;


DECLARE_FAKE_VALUE_FUNC(DisplayError_t, Display_Send, DisplayCmd_t);

DECLARE_FAKE_VALUE_FUNC(DisplayError_t, Display_Init);

DECLARE_FAKE_VALUE_FUNC(DisplayError_t, Display_Reset);

DECLARE_FAKE_VALUE_FUNC(DisplayError_t, Display_Error, error_code_t);

DECLARE_FAKE_VALUE_FUNC(DisplayError_t, Display_Evac, uint8_t, uint32_t);

#endif
#endif
