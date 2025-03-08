/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file Display.c
 * @brief Function implementations for the display driver.
 *
 * This contains functions relevant to sending/receiving messages
 * to/from our Nextion display.
 * 
 */

#include "Display.h"
#include "bsp.h"   // for writing to UART
#include "Tasks.h" // for os and fault error codes

#include "ReadCarCAN.h"
#include "UpdateDisplay.h"
#include "ReadTritium.h"

// Assignment commands have only 1 arg, an operator, and an attribute
#define IS_ASSIGN_CMD(cmd) (cmd.compOrCmd != NULL && cmd.op != NULL && cmd.attr != NULL && cmd.numArgs == 1)
// Operational commands have no attribute and no operator, just a command and >= 0 arguments
#define IS_OP_CMD(cmd) (cmd.op == NULL && cmd.attr == NULL)

static const char *TERMINATOR = "\xff\xff\xff";

const char* DISPLAY_COMP_STR[NUM_COMPONENTS] = {
	// Boolean components
	"arr",
	"hb",
	"cs",
	"mcs",
	"brake",
	"mot",
	// Non-boolean components
	"vel",
	"accel",
	"soc",
	"supp",
	"cruiseSt",
	"rbsSt",
	"pv",
	"pc",
	"pt",
	"mcv",
	"mcc",
	"heatsink",
	"gear",
	// Fault code components
	"oserr",
	"faulterr"
};

DisplayError_t Display_Init(){
	BSP_UART_Init(DISP_OUT);
	
	return Display_Reset();
}

DisplayError_t Display_Send(DisplayCmd_t cmd) {
	char msgArgs[MAX_MSG_LEN];
	
	if (IS_ASSIGN_CMD(cmd)) {
		if (cmd.argTypes[0] == INT_ARG){
			sprintf(msgArgs, "%d", (int)cmd.args[0].num);
		}
		else { // STR_ARG
			if (cmd.args[0].str == NULL) return DISPLAY_ERR_PARSE;
			sprintf(msgArgs, "%s", cmd.args[0].str);
		}

		BSP_UART_Write(DISP_OUT, cmd.compOrCmd, strlen(cmd.compOrCmd));
		BSP_UART_Write(DISP_OUT, ".", 1);
		BSP_UART_Write(DISP_OUT, cmd.attr, strlen(cmd.attr));
		BSP_UART_Write(DISP_OUT, cmd.op, strlen(cmd.op));
	}
	else if (IS_OP_CMD(cmd)) {
		if (cmd.numArgs > MAX_ARGS) return DISPLAY_ERR_OTHER;

		msgArgs[0] = ' '; // No args
		msgArgs[1] = '\0';
		if (cmd.numArgs >= 1) { // If there are arguments
			for (int i = 0; i < cmd.numArgs; i++) {
				char arg[MAX_ARG_LEN];
				if (cmd.argTypes[i] == INT_ARG) {
					sprintf(arg, "%d", (int)cmd.args[i].num);
				}
				else { // STR_ARG
					if (cmd.args[i].str == NULL) return DISPLAY_ERR_PARSE;
					sprintf(arg, "%s", cmd.args[i].str);
				}

				strcat(msgArgs, arg);

				if (i < cmd.numArgs - 1) { // delimiter
					strcat(msgArgs, ",");
				}
			}
		}

		BSP_UART_Write(DISP_OUT, cmd.compOrCmd, strlen(cmd.compOrCmd));
	}
	else { // Error parsing command struct
		return DISPLAY_ERR_PARSE;
	}

	if (cmd.numArgs >= 1) { // If there are arguments
		BSP_UART_Write(DISP_OUT, msgArgs, strlen(msgArgs));
	}

	BSP_UART_Write(DISP_OUT, (char *)TERMINATOR, strlen(TERMINATOR));

	return DISPLAY_ERR_NONE;
}

DisplayError_t Display_Reset(){
	DisplayCmd_t restCmd = {
		.compOrCmd = "rest",
		.attr = NULL,
		.op = NULL,
		.numArgs = 0
	};

	BSP_UART_Write(DISP_OUT, (char *)TERMINATOR, strlen(TERMINATOR)); // Terminates any in progress command

	return Display_Send(restCmd);
}

DisplayError_t Display_Error() {
	BSP_UART_Write(DISP_OUT, (char *)TERMINATOR, strlen(TERMINATOR)); // Terminates any in progress command

	// Switch to fault page 
	DisplayCmd_t err_pg_cmd = {
		.compOrCmd = "page",
		.attr = NULL,
		.op = NULL,
		.numArgs = 1,
		.argTypes = {INT_ARG},
		{{.num=FAULT}}
	};
	Display_Send(err_pg_cmd);

	// Display OS error if there is one
	if (Error_OS != OS_ERR_NONE) {
		DisplayCmd_t os_flt_cmd = {
			.compOrCmd = (char*) DISPLAY_COMP_STR[19], // "oserr"
			.attr = "txt",
			.op = "=",
			.numArgs = 1,
			.argTypes = {STR_ARG},
			{{.str=ErrMsg_OS}}
		};
		Display_Send(os_flt_cmd);
		strncpy(ErrMsg_OS, "\"N/A\"", 6);
		memset(&Error_OS, 0, sizeof(error_code_t));
	}
	
	// Display other errors if there are any.
	// Prioritized errors in order of importance:
	// 1. ReadTritium
	// 2. ReadCarCAN
	// 3. UpdateDisplay
	if (Error_ReadTritium != T_NONE) {
		DisplayCmd_t moco_flt_cmd = {
			.compOrCmd = (char*) DISPLAY_COMP_STR[20], // "faulterr"
			.attr = "txt",
			.op = "=",
			.numArgs = 1,
			.argTypes = {STR_ARG},
			{{.str=ErrMsg_ReadTritium}}
		};
		Display_Send(moco_flt_cmd);
		strncpy(ErrMsg_ReadTritium, "\"N/A\"", 6);
		memset(&Error_ReadTritium, 0, sizeof(error_code_t));
	}
	else if (Error_ReadCarCAN != READCARCAN_ERR_NONE) {
		DisplayCmd_t rcc_flt_cmd = {
			.compOrCmd = (char*) DISPLAY_COMP_STR[20], // "faulterr"
			.attr = "txt",
			.op = "=",
			.numArgs = 1,
			.argTypes = {STR_ARG},
			{{.str=ErrMsg_ReadCarCAN}}
		};
		Display_Send(rcc_flt_cmd);
		strncpy(ErrMsg_ReadCarCAN, "\"N/A\"", 6);
		memset(&Error_ReadCarCAN, 0, sizeof(error_code_t));
	}
	else if (Error_UpdateDisplay != UPDATEDISPLAY_ERR_NONE) {
		DisplayCmd_t disp_flt_cmd = {
			.compOrCmd = (char*) DISPLAY_COMP_STR[20], // "faulterr"
			.attr = "txt",
			.op = "=",
			.numArgs = 1,
			.argTypes = {STR_ARG},
			{{.str=ErrMsg_UpdateDisplay}}
		};
		Display_Send(disp_flt_cmd);
		strncpy(ErrMsg_UpdateDisplay, "\"N/A\"", 6);
		memset(&Error_UpdateDisplay, 0, sizeof(error_code_t));
	}


	// char faultPage[7] = "page 2";
	// BSP_UART_Write(DISP_OUT, faultPage, strlen(faultPage));
	// BSP_UART_Write(DISP_OUT, (char *)TERMINATOR, strlen(TERMINATOR));

	// char setFaultCode[20];
	// char* setFaultMsg = "";
	// char* faultMsg = "";
	

	// Combine two strings (code and message) before sending to display, in order to use one UART write
	// OS Error
	// sprintf(setFaultCode, "%s%d\n", "oserr.val=", (uint16_t) 0x12);
	// sprintf(setFaultMsg, "%s", (char *)ErrMsg_OS);
	// faultMsg = strcat(setFaultCode, setFaultMsg);
	// BSP_UART_Write(DISP_OUT, faultMsg, strlen(faultMsg));
	// BSP_UART_Write(DISP_OUT, (char *)TERMINATOR, strlen(TERMINATOR));
	// memset(setFaultCode, 0, strlen(setFaultCode) * sizeof(char));
	// memset(setFaultMsg, 0, strlen(setFaultMsg) * sizeof(char));


	// ReadCarCAN Error
	// sprintf(setFaultCode, "%s%d\n", "rccerr.val=", (uint16_t) 0x12);
	// sprintf(setFaultMsg, "%s", (char *)ErrMsg_ReadCarCAN);
	// faultMsg = strcat(setFaultCode, setFaultMsg);
	// BSP_UART_Write(DISP_OUT, faultMsg, strlen(faultMsg));
	// memset(setFaultCode, 0, strlen(setFaultCode) * sizeof(char));
	// memset(setFaultMsg, 0, strlen(setFaultMsg) * sizeof(char));
	// BSP_UART_Write(DISP_OUT, (char *)TERMINATOR, strlen(TERMINATOR));


	// // Motor Controller Error
	// sprintf(setFaultCode, "%s%d\n", "merr.val=", (uint16_t) 0x12);
	// sprintf(setFaultMsg, "%s", (char *)ErrMsg_ReadTritium);
	// faultMsg = strcat(setFaultCode, setFaultMsg);
	// BSP_UART_Write(DISP_OUT, faultMsg, strlen(faultMsg));
	// memset(setFaultCode, 0, strlen(setFaultCode) * sizeof(char));
	// memset(setFaultMsg, 0, strlen(setFaultMsg) * sizeof(char));
	// BSP_UART_Write(DISP_OUT, (char *)TERMINATOR, strlen(TERMINATOR));


	// // Display Error
	// // TODO: Display error messages are taken care in UpdateDisplay?
	// sprintf(setFaultCode, "%s%d", "disperr.val=", (uint16_t) 0x12);
	// BSP_UART_Write(DISP_OUT, setFaultCode, strlen(setFaultCode));
	// memset(setFaultCode, 0x12, strlen(setFaultCode) * sizeof(char));
	// BSP_UART_Write(DISP_OUT, (char *)TERMINATOR, strlen(TERMINATOR));

	return DISPLAY_ERR_NONE;
}

DisplayError_t Display_Evac(uint8_t SOC_percent, uint32_t supp_mv) {
	BSP_UART_Write(DISP_OUT, (char *)TERMINATOR, strlen(TERMINATOR)); // Terminates any in progress command

	// Switch to evac page 
	DisplayCmd_t evac_pg_cmd = {
		.compOrCmd = "page",
		.attr = NULL,
		.op = NULL,
		.numArgs = 1,
		.argTypes = {INT_ARG},
		{{.num=EVAC}}
	};
	Display_Send(evac_pg_cmd);

	// Send SOC and SBPV values
	DisplayCmd_t soc_cmd = {
		.compOrCmd = (char*) DISPLAY_COMP_STR[8], // "soc"
		.attr = "val",
		.op = NULL,
		.numArgs = 1,
		.argTypes = {INT_ARG},
		{{.num=SOC_percent}}
	};
	Display_Send(soc_cmd);

	DisplayCmd_t supp_cmd = {
		.compOrCmd = (char*) DISPLAY_COMP_STR[9], // "supp"
		.attr = "val",
		.op = NULL,
		.numArgs = 1,
		.argTypes = {INT_ARG},
		{{.num=supp_mv}}
	};
	Display_Send(supp_cmd);
	
	// char evacPage[7] = "page 3";
	// BSP_UART_Write(DISP_OUT, evacPage, strlen(evacPage));
	// BSP_UART_Write(DISP_OUT, (char *)TERMINATOR, strlen(TERMINATOR));

	// char soc[13];
	// sprintf(soc, "%s%d", "soc.val=", (int)SOC_percent);
	// BSP_UART_Write(DISP_OUT, soc, strlen(soc));
	// BSP_UART_Write(DISP_OUT, (char *)TERMINATOR, strlen(TERMINATOR));

	// char supp[18];
	// sprintf(supp, "%s%d", "supp.val=", (int)supp_mv);
	// BSP_UART_Write(DISP_OUT, supp, strlen(supp));
	// BSP_UART_Write(DISP_OUT, (char *)TERMINATOR, strlen(TERMINATOR));

	return DISPLAY_ERR_NONE;
}
