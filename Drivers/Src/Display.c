/** Copyright (c) 2022 UT Longhorn Racing Solar
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
#include "bsp.h"	// for writing to UART
#include "Tasks.h"	// for os and fault error codes

#define DISP_OUT UART_3
static const char *TERMINATOR = "\xff\xff\xff";

Display_Error_t Display_Init(){
	BSP_UART_Init(DISP_OUT);

	char buf[8];	// Read buffer
	BSP_UART_Read(DISP_OUT, (char*)buf);

	return Display_Reset();
}

Display_Error_t Display_Send(Display_Cmd_t cmd){
	char msgArgs[32];
	if(cmd.compOrCmd != NULL && cmd.op != NULL && cmd.attr != NULL && cmd.numArgs == 1){	// Assignment commands have only 1 arg, an operator, and an attribute
		if(cmd.argTypes[0] == INT_ARG){
			sprintf(msgArgs, "%d", (int)cmd.args[0].num);
		}else{
			if (cmd.args[0].str == NULL) return DISPLAY_ERR_PARSE;
			sprintf(msgArgs, "%s", cmd.args[0].str);
		}
		
		BSP_UART_Write(DISP_OUT, cmd.compOrCmd, strlen(cmd.compOrCmd));
		BSP_UART_Write(DISP_OUT, ".", 1);
		BSP_UART_Write(DISP_OUT, cmd.attr, strlen(cmd.attr));
		BSP_UART_Write(DISP_OUT, cmd.op, strlen(cmd.op));
	}
	else if(cmd.op == NULL && cmd.attr == NULL){	// Operational commands have no attribute and no operator, just a command and >= 0 arguments
		msgArgs[0] = ' '; // No args
		msgArgs[1] = '\0';
		if (cmd.numArgs > MAX_ARGS) return DISPLAY_ERR_OTHER;
		if(cmd.numArgs >= 1){	// If there are arguments
			for(int i=0; i<cmd.numArgs; i++){
				char arg[16];
				if(cmd.argTypes[i] == INT_ARG){
					sprintf(arg, "%d", (int)cmd.args[i].num);
				}
				else{
					sprintf(arg, "%s", cmd.args[i].str);
				}

				strcat(msgArgs, arg);

				if(i<cmd.numArgs-1){	// delimiter
					strcat(msgArgs, ",");
				}
			}
		}
		BSP_UART_Write(DISP_OUT, cmd.compOrCmd, strlen(cmd.compOrCmd));

	} else{	// Error parsing command struct
		return DISPLAY_ERR_PARSE;
	}
	
	if(cmd.numArgs >= 1){	// If there are arguments
		BSP_UART_Write(DISP_OUT, msgArgs, strlen(msgArgs));
	}
	
	BSP_UART_Write(DISP_OUT, (char*)TERMINATOR, strlen(TERMINATOR));

	char buf[8];
	BSP_UART_Read(DISP_OUT, buf);

	if (strcmp(cmd.compOrCmd, "rest") == 0) return DISPLAY_ERR_NONE;
	
	// Error validation
	switch(buf[4]){	// buf[4]: first byte of second half of response
		case 0x00:
			return DISPLAY_ERR_INV_INSTR;
		case 0x01:
			return DISPLAY_ERR_NONE;
		case 0x02:
			return DISPLAY_ERR_INV_COMP;
		case 0x03:
			return DISPLAY_ERR_INV_PGID;
		case 0x1A:
			return DISPLAY_ERR_INV_VAR;
		case 0x1B:
			return DISPLAY_ERR_INV_VAROP;
		case 0x1C:
			return DISPLAY_ERR_ASSIGN;
		case 0x1E:
			return DISPLAY_ERR_PARAMS;
		default:
			return DISPLAY_ERR_OTHER;
	}
}

Display_Error_t Display_Reset(){
	Display_Cmd_t restCmd = {
		.compOrCmd = "rest",
		.attr = NULL,
		.op = NULL,
		.numArgs = 0
	};

	BSP_UART_Write(DISP_OUT, (char*)TERMINATOR, strlen(TERMINATOR));	// Terminates any in progress command

	return Display_Send(restCmd);
}

Display_Error_t Display_Fault(os_error_loc_t osErrCode, fault_bitmap_t faultCode){
	BSP_UART_Write(DISP_OUT, (char*)TERMINATOR, strlen(TERMINATOR));	// Terminates any in progress command

	char faultPage[7] = "page 2";
	BSP_UART_Write(DISP_OUT, faultPage, strlen(faultPage));
	BSP_UART_Write(DISP_OUT, (char*)TERMINATOR, strlen(TERMINATOR));

	char setOSCode[17];
	sprintf(setOSCode, "%s\"%04x\"", "oserr.txt=", (uint16_t)osErrCode);
	BSP_UART_Write(DISP_OUT, setOSCode, strlen(setOSCode));
	BSP_UART_Write(DISP_OUT, (char*)TERMINATOR, strlen(TERMINATOR));

	char setFaultCode[18];
	sprintf(setFaultCode, "%s\"%02x\"", "faulterr.txt=", (uint8_t)faultCode);
	BSP_UART_Write(DISP_OUT, setFaultCode, strlen(setFaultCode));
	BSP_UART_Write(DISP_OUT, (char*)TERMINATOR, strlen(TERMINATOR));

	return DISPLAY_ERR_NONE;
}

void assertDisplayError(Display_Error_t err){
	OS_ERR os_err;

	if(err != DISPLAY_ERR_NONE){
		FaultBitmap |= FAULT_DISPLAY;

		OSSemPost(&FaultState_Sem4, OS_OPT_POST_1, &os_err);
		assertOSError(OS_DISPLAY_LOC, os_err);
	}
}