/**
 * @file Display.h
 * @brief The display driver is responsible for all interactions with the display. 
 * It includes functions to reset the display and to send commands to it. 
 * It also defines a display command struct, from which a command string is assembled 
 * and then sent out to the display.
 * 
 */

#ifndef __DISPLAY_H
#define __DISPLAY_H

#include "common.h"	// common headers
#include "Tasks.h"	// for os and fault error locs

/**
 * @def MAX_ARGS
 * @brief maximum number of arguments in a command packet
 */
#define MAX_ARGS 2

/**
 * @enum DisplayError_t
 * @brief Error codes for display driver
 * @note Currently only ERR_NONE and ERR_PARSE are used, since there is a bug
 * where the nextion does not return errors properly to us.
 */
typedef enum{
    /* No error */
	DISPLAY_ERR_NONE,
	/* Error parsing command struct passed to Display_Send */
    DISPLAY_ERR_PARSE,	    
	/* Invalid instruction passed to nextion (0x00) */
    DISPLAY_ERR_INV_INSTR,	
	/* Invalid component id passed to nextion (0x02) */
    DISPLAY_ERR_INV_COMP,	
	/* Invalid page id passed to nextion	(0x03) */
    DISPLAY_ERR_INV_PGID,	
	/* Invalid variable name passed to nextion	(0x1A) */
    DISPLAY_ERR_INV_VAR,	
	/* Invalid variable operation passed to nextion	(0x1B) */
    DISPLAY_ERR_INV_VAROP,	
	/* Assignment failure nextion	(0x1C) */
    DISPLAY_ERR_ASSIGN,	    
	/* Invalid number of parameters passed to nextion	(0x1E) */
    DISPLAY_ERR_PARAMS,	    
	/* Command arg list exceeded MAX_ARGS elements */
    DISPLAY_ERR_MAX_ARGS,   
	/* Other nextion display error */
    DISPLAY_ERR_OTHER       
} DisplayError_t;


/**
 * @def Page_t
 * @brief All three pages on the HMI 
 */
typedef enum{
	STARTUP	=0,
	INFO,
	FAULT
} Page_t;

/**
 * @def Arg_e
 * @brief Argument types
 */
typedef enum{
	STR_ARG,
	INT_ARG
} Arg_e;

/**
 * @struct DisplayCmd_t
 * @brief Packages relevant display command data
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
 * @param faultCode the application's fault code (will be displayed in hex)
 * @returns DisplayError_t
 */
DisplayError_t Display_Error(error_code_t faultCode);

/**
 * @brief Overwrites any processing commands and triggers the evacuation screen
 * @param SOC_percent the state of charge of the battery in percent
 * @param supp_mv the voltage of the battery in millivolts
 * @returns DisplayError_t
*/
DisplayError_t Display_Evac(uint8_t SOC_percent, uint32_t supp_mv);

#endif



