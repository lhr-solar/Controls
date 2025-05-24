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

#include "common.h"


// #define DISP_OUT DISPLAY
#define MAX_MSG_LEN 32
#define MAX_ARG_LEN 16

#define MAX_ARGS 2	// maximum # of arguments in a command packet

/**
 * Enum and corresponding array for easy component selection.
 */
typedef enum{
	// Boolean components
	DISP_HEARTBEAT=0,
	DISP_PACK_CURR_SIGN,
	DISP_MC_CURR_SIGN,
	DISP_BRAKE,
	DISP_BLINK,
	// Non-boolean components
	DISP_ARRAY_EN,
	DISP_ARRAY_PC,
	DISP_MOTOR_EN,
	DISP_MOTOR_PC,
	DISP_VELOCITY,
	DISP_ACCEL_METER,
	DISP_SOC,
	DISP_SUPP_BATT,
	DISP_CRUISE_ST,
	DISP_REGEN_ST,
	DISP_PACK_VOLTAGE,
	DISP_PACK_CURRENT,
	DISP_PACK_TEMP,
	DISP_MC_BUS_VOLTAGE,
	DISP_MC_BUS_CURRENT,
	DISP_HEAT_SINK_TEMP,
	DISP_GEAR,
	// Fault code components
	DISP_OS_CODE,
	DISP_FAULT_CODE,
	DISP_EVAC_MSG,
	DISP_NUM_COMPONENTS
} Component_t;

/**
 * Values corresponding to the component enum.
 */
extern uint32_t g_display_comp_vals[DISP_NUM_COMPONENTS];
extern const char *DISPLAY_COMP_STR[DISP_NUM_COMPONENTS];

/**
 * Error types
 */
typedef enum{ // Currently only ERR_NONE and ERR_PARSE are used
	DISPLAY_ERR_NONE 		= 0x0,		// No Error
	DISPLAY_ERR_PARSE		= 1,		// Error parsing command struct passed to Display_Send
	DISPLAY_ERR_OTHER		= 1 << 1,	// Other nextion display error

	// All unused
	// DISPLAY_ERR_INV_INSTR	= 1 << 2,	// Invalid instruction passed to nextion (0x00)
	// DISPLAY_ERR_INV_COMP	= 1 << 3,	// Invalid component id passed to nextion (0x02)
	// DISPLAY_ERR_INV_PGID 	= 1 << 4,	// Invalid page id passed to nextion	(0x03)
	// DISPLAY_ERR_INV_VAR		= 1 << 5,	// Invalid variable name passed to nextion	(0x1A)
	// DISPLAY_ERR_INV_VAROP	= 1 << 6,	// Invalid variable operation passed to nextion	(0x1B)
	// DISPLAY_ERR_ASSIGN		= 1 << 7,	// Assignment failure nextion	(0x1C)
	// DISPLAY_ERR_PARAMS		= 1 << 8,	// Invalid number of parameters passed to nextion	(0x1E)
	// DISPLAY_ERR_MAX_ARGS	= 1 << 9    // Command arg list exceeded MAX_ARGS elements
} DisplayError_t;


/**
 * All three pages on the HMI 
 */
typedef enum Page {
	STARTUP	= (uint32_t) 0,
	INFO 	= (uint32_t) 1,
	FAULT 	= (uint32_t) 2,
	EVAC    = (uint32_t) 3
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
DisplayError_t Display_Error(void);

/**
 * @brief Overwrites any processing commands and triggers the evacuation screen
 * @param SOC_percent the state of charge of the battery in percent
 * @param supp_mv the voltage of the battery in millivolts
 * @returns DisplayError_t
 */
DisplayError_t Display_Evac(uint8_t SOC_percent, uint32_t supp_mv);

/**
 * @brief Changes the page of the display
 * @returns DisplayError_t
 */
DisplayError_t Display_SetPage(Page_t page);

DisplayError_t Display_Refresh(void); 

#endif


/* @} */
