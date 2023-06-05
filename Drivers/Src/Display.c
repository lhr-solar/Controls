/**
 * @copyright Copyright (c) 2022 UT Longhorn Racing Solar
 * @file Display.c
 * @brief Function implementations for the display driver.
 *
 * This contains functions relevant to sending/receiving messages
 * to/from our Nextion display.
 *
 * @author Ishan Deshpande (IshDeshpa)
 * @author Roie Gal (Cam0Cow)
 * @author Nathaniel Delgado (NathanielDelgado)
 */

#include "Display.h"
#include "bsp.h"   // for writing to UART
#include "Tasks.h" // for os and fault error codes
#include "FaultState.h"

#define DISP_OUT UART_3
#define MAX_MSG_LEN 32
#define MAX_ARG_LEN 16
// Assignment commands have only 1 arg, an operator, and an attribute
#define isAssignCmd(cmd) (cmd.compOrCmd != NULL && cmd.op != NULL && cmd.attr != NULL && cmd.numArgs == 1)
// Operational commands have no attribute and no operator, just a command and >= 0 arguments
#define isOpCmd(cmd) (cmd.op == NULL && cmd.attr == NULL)

#define RESTART_THRESHOLD 3 // number of times to restart before fault


static const char *TERMINATOR = "\xff\xff\xff";

DisplayError_t Display_Init(){
	BSP_UART_Init(DISP_OUT);
	return Display_Reset();
}

DisplayError_t Display_Send(DisplayCmd_t cmd){
	char msgArgs[MAX_MSG_LEN];
	if (isAssignCmd(cmd)){
		if (cmd.argTypes[0] == INT_ARG){
			sprintf(msgArgs, "%d", (int)cmd.args[0].num);
		}
		else{
			if (cmd.args[0].str == NULL){return DISPLAY_ERR_PARSE;}
			sprintf(msgArgs, "%s", cmd.args[0].str);
		}

		BSP_UART_Write(DISP_OUT, cmd.compOrCmd, strlen(cmd.compOrCmd));
		BSP_UART_Write(DISP_OUT, ".", 1);
		BSP_UART_Write(DISP_OUT, cmd.attr, strlen(cmd.attr));
		BSP_UART_Write(DISP_OUT, cmd.op, strlen(cmd.op));
	}
	else if (isOpCmd(cmd)){
		msgArgs[0] = ' '; // No args
		msgArgs[1] = '\0';
		if (cmd.numArgs > MAX_ARGS){return DISPLAY_ERR_OTHER;}
		if (cmd.numArgs >= 1){ // If there are arguments
			for (int i = 0; i < cmd.numArgs; i++){
				char arg[MAX_ARG_LEN];
				if (cmd.argTypes[i] == INT_ARG){
					sprintf(arg, "%d", (int)cmd.args[i].num);
				}
				else{
					sprintf(arg, "%s", cmd.args[i].str);
				}

				strcat(msgArgs, arg);

				if (i < cmd.numArgs - 1){ // delimiter
					strcat(msgArgs, ",");
				}
			}
		}
		BSP_UART_Write(DISP_OUT, cmd.compOrCmd, strlen(cmd.compOrCmd));
	}
	else{ // Error parsing command struct
		return DISPLAY_ERR_PARSE;
	}

	if (cmd.numArgs >= 1){ // If there are arguments
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
		.numArgs = 0};

	BSP_UART_Write(DISP_OUT, (char *)TERMINATOR, strlen(TERMINATOR)); // Terminates any in progress command

	return Display_Send(restCmd);
}

DisplayError_t Display_Fault(os_error_loc_t osErrCode, fault_bitmap_t faultCode){
	BSP_UART_Write(DISP_OUT, (char *)TERMINATOR, strlen(TERMINATOR)); // Terminates any in progress command

	char faultPage[7] = "page 2";
	BSP_UART_Write(DISP_OUT, faultPage, strlen(faultPage));
	BSP_UART_Write(DISP_OUT, (char *)TERMINATOR, strlen(TERMINATOR));

	char setOSCode[17];
	sprintf(setOSCode, "%s\"%04x\"", "oserr.txt=", (uint16_t)osErrCode);
	BSP_UART_Write(DISP_OUT, setOSCode, strlen(setOSCode));
	BSP_UART_Write(DISP_OUT, (char *)TERMINATOR, strlen(TERMINATOR));

	char setFaultCode[18];
	sprintf(setFaultCode, "%s\"%02x\"", "faulterr.txt=", (uint8_t)faultCode);
	BSP_UART_Write(DISP_OUT, setFaultCode, strlen(setFaultCode));
	BSP_UART_Write(DISP_OUT, (char *)TERMINATOR, strlen(TERMINATOR));

	return DISPLAY_ERR_NONE;
}

DisplayError_t Display_Evac(uint8_t SOC_percent, uint32_t supp_mv){
	BSP_UART_Write(DISP_OUT, (char *)TERMINATOR, strlen(TERMINATOR)); // Terminates any in progress command

	char evacPage[7] = "page 3";
	BSP_UART_Write(DISP_OUT, evacPage, strlen(evacPage));
	BSP_UART_Write(DISP_OUT, (char *)TERMINATOR, strlen(TERMINATOR));

	char soc[13];
	sprintf(soc, "%s%d", "soc.val=", (int)SOC_percent);
	BSP_UART_Write(DISP_OUT, soc, strlen(soc));
	BSP_UART_Write(DISP_OUT, (char *)TERMINATOR, strlen(TERMINATOR));

	char supp[18];
	sprintf(supp, "%s%d", "supp.val=", (int)supp_mv);
	BSP_UART_Write(DISP_OUT, supp, strlen(supp));
	BSP_UART_Write(DISP_OUT, (char *)TERMINATOR, strlen(TERMINATOR));

	return DISPLAY_ERR_NONE;
}

static void callback_displayError(void){
	static uint8_t disp_fault_cnt = 0; // If faults > three times total, Display_Fault is called
        if(disp_fault_cnt>RESTART_THRESHOLD){ 
            Display_Fault(OSErrLocBitmap, FaultBitmap); 
        } else { 
            disp_fault_cnt++; 
            Display_Reset(); 
            return; 
        }  
}

void assertDisplayError(DisplayError_t err){
	if (err != DISPLAY_ERR_NONE){
		exception_t displayError = {2, "display error", &callback_displayError};
		assertExceptionError(displayError);
	}
}
