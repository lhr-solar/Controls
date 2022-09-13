/** Copyright (c) 2022 UT Longhorn Racing Solar
 * @file Display.c
 * @brief Function implementations for the display driver.
 * 
 * This contains functions relevant to sending/receiving messages
 * to/from our Nextion display.
 * 
 * @author Ishan Deshpande (IshDeshpa)
 * @author Roie Gal (Cam0Cow)
*/

#include "Display.h"
#include "bsp.h"	// for writing to UART
#include "Tasks.h"	// for os and fault error codes

#define DISP_OUT UART_3
static const char *TERMINATOR = "\xff\xff\xff";

ErrorStatus Display_Init(){
	BSP_UART_Init(DISP_OUT);
	return SUCCESS;
}

ErrorStatus Display_Send(Display_Cmd_t cmd){
	char msgArgs[16];
	if((cmd.numArgs == 1 && cmd.args != NULL && cmd.argTypes != NULL) && cmd.op != NULL && cmd.attr != NULL){	// Assignment commands have only 1 arg, an operator, and an attribute
		if(cmd.argTypes[0]){
			sprintf(msgArgs, "%d", (int)cmd.args[0].num);
		}
		else{
			sprintf(msgArgs, "%s", cmd.args[0].str);
		}
		
		BSP_UART_Write(DISP_OUT, cmd.compOrCmd, strlen(cmd.compOrCmd));
		BSP_UART_Write(DISP_OUT, ".", strlen(cmd.compOrCmd));
		BSP_UART_Write(DISP_OUT, cmd.attr, strlen(cmd.attr));
		BSP_UART_Write(DISP_OUT, cmd.op, strlen(cmd.op));
	}
	else if(cmd.op == NULL && cmd.attr == NULL){	// Operational commands have no attribute and no operator, just a command and >= 0 arguments
		sprintf(msgArgs, " ");
		if(cmd.numArgs >= 1 && cmd.argTypes != NULL && cmd.args != NULL){	// If there are arguments
			for(int i=0; i<cmd.numArgs; i++){
				char arg[16];
				if(cmd.argTypes[i]){
					sprintf(arg, "%d", (int)cmd.args[i].num);
				}
				else{
					sprintf(arg, "%s", cmd.args[i].str);
				}

				strcat(msgArgs, arg);

				if(i<cmd.numArgs-1){	// delimiter
					strcat(msgArgs, ", ");
				}
			}
		}
		
		BSP_UART_Write(DISP_OUT, cmd.compOrCmd, strlen(cmd.compOrCmd));
	}
	else{	// Invalid command
		return ERROR;
	}
	
	BSP_UART_Write(DISP_OUT, msgArgs, strlen(msgArgs));
	BSP_UART_Write(DISP_OUT, (char*)TERMINATOR, strlen(TERMINATOR));

	char buf[8];
	BSP_UART_Read(DISP_OUT, buf);
	if(buf[0] == 0x01){	// Successful command
		return SUCCESS;
	}
	return ERROR;
}

ErrorStatus Display_Reset(){
	Display_Cmd_t restCmd = {
		.compOrCmd = "rest",
		.attr = NULL,
		.op = NULL,
		.numArgs = 0
	};

	BSP_UART_Write(DISP_OUT, (char*)TERMINATOR, strlen(TERMINATOR));	// Terminates any in progress command

	return Display_Send(restCmd);
}

ErrorStatus Display_Fault(os_error_loc_t osErrCode, fault_bitmap_t faultCode){
	BSP_UART_Write(DISP_OUT, (char*)TERMINATOR, strlen(TERMINATOR));	// Terminates any in progress command

	char faultPage[7] = "page 2";
	BSP_UART_Write(DISP_OUT, faultPage, strlen(faultPage));
	BSP_UART_Write(DISP_OUT, (char*)TERMINATOR, strlen(TERMINATOR));

	char setOSCode[15];
	sprintf(setOSCode, "%s%04X", "oserr.txt=", (uint16_t)osErrCode);
	BSP_UART_Write(DISP_OUT, setOSCode, strlen(setOSCode));
	BSP_UART_Write(DISP_OUT, (char*)TERMINATOR, strlen(TERMINATOR));

	char setFaultCode[16];
	sprintf(setFaultCode, "%s%02X", "faulterr.txt=", (uint8_t)faultCode);
	BSP_UART_Write(DISP_OUT, setFaultCode, strlen(setFaultCode));
	BSP_UART_Write(DISP_OUT, (char*)TERMINATOR, strlen(TERMINATOR));

	return SUCCESS;
}