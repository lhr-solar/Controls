#include "bsp.h"
#include "Display.h"
#include "Tasks.h"

#define FIFO_TYPE Display_Cmd_t
#define FIFO_SIZE sizeof(Display_Cmd_t)*5
#define FIFO_NAME disp_fifo
#include "fifo.h"

disp_fifo_t msg_queue;
static OS_SEM DisplayQ_Sem4;
static OS_MUTEX DisplayQ_Mutex;

#define DISP_OUT UART_3
static const char *TERMINATOR = "\xff\xff\xff";

// Should make an enum for the gears

typedef enum{
	// Boolean components
	LEFT=0,
	HEAD,
	RIGHT,
	HZD,
	CRUISE,
	REGEN,
	ARRAY,
	MOTOR,
	// Non-boolean components
	VELOCITY,
	ACCEL_METER,
	SOC,
	SUPP_BATT,
	SOC_BAR,
	SUPP_BATT_BAR,
	// Fault code components
	OS_CODE,
	FAULT_CODE
} Component_t;

const char* compStrings[17]= {
	// Boolean components
	"ltime",
	"head",
	"rtime",
	"hzd"
	"cruise",
	"rbs",
	"arr",
	"mot",
	"BREAKLIGHTS?",
	// Non-boolean components
	"vel",
	"accel",
	"soc",
	"supp",
	"socbar",
	"suppbar",
	// Fault code components
	"oserr",
	"faulterr"
};

ErrorStatus Display_PutNext(Display_Cmd_t cmd){
	CPU_TS ticks;
	OS_ERR err;

	OSMutexPend(&DisplayQ_Mutex, 0, OS_OPT_POST_NONE, &ticks, &err);
  	assertOSError(OS_DISPLAY_LOC, err);  
	
	bool success = disp_fifo_put(&msg_queue, cmd);

	OSMutexPost(&DisplayQ_Mutex, OS_OPT_POST_NONE, &err);
	assertOSError(OS_DISPLAY_LOC, err);

	if(success){
		OSSemPost(&DisplayQ_Sem4, OS_OPT_POST_NONE, &err);
		assertOSError(OS_DISPLAY_LOC, err);
	}

	return success ? SUCCESS : ERROR;
}

ErrorStatus Display_SendNext(){
	Display_Cmd_t cmd;

	OS_ERR err;
	CPU_TS ticks;
    
	OSSemPend(&DisplayQ_Sem4, 0, OS_OPT_PEND_BLOCKING, &ticks, &err);
	assertOSError(OS_DISPLAY_LOC, err);
	
	OSMutexPend(&DisplayQ_Mutex, 0, OS_OPT_POST_NONE, &ticks, &err);
	assertOSError(OS_DISPLAY_LOC, err);
	
	bool result = disp_fifo_get(&msg_queue, &cmd);
	OSMutexPost(&DisplayQ_Mutex, OS_OPT_POST_NONE, &err);
	assertOSError(OS_SEND_CAN_LOC, err);
	
	if(result == ERROR)
		return ERROR;
	
	char msg[32];
	if((cmd.numArgs == 1 && cmd.args != NULL && cmd.argTypes != NULL) && cmd.op != NULL && cmd.attr != NULL){	// Assignment commands have only 1 arg, an operator, and an attribute
		if(cmd.argTypes[0]){
			sprintf(msg, "%s.%s%s%d", cmd.compOrCmd, cmd.attr, cmd.op, cmd.args[0].num);
		}
		else{
			sprintf(msg, "%s.%s%s%s", cmd.compOrCmd, cmd.attr, cmd.op, cmd.args[0].str);
		}
	}
	else if(cmd.op == NULL && cmd.attr == NULL){	// Operational commands have no attribute and no operator, just a command and >= 0 arguments
		sprintf(msg, "%s", cmd.compOrCmd);
		if(cmd.numArgs == 1 && cmd.argTypes != NULL && cmd.args != NULL){	// If there are arguments
			strcat(msg, " ");
			for(int i=0; i<cmd.numArgs; i++){
				char arg[16];
				if(cmd.argTypes[i]){
					sprintf(arg, "%d", cmd.args[i].num);
				}
				else{
					sprintf(arg, "%s", cmd.args[i].str);
				}

				strcat(msg, arg);

				if(i<cmd.numArgs-1){	// delimiter
					strcat(msg, ",");
				}
			}
		}
	}
	else{	// Invalid command
		return ERROR;
	}
	
	BSP_UART_Write(DISP_OUT, msg, strlen(msg));
	BSP_UART_Write(DISP_OUT, (char*)TERMINATOR, strlen(TERMINATOR));

	char buf[8];
	BSP_UART_Read(DISP_OUT, buf);
	if(buf[0] == 0x01){	// Successful command
		return SUCCESS;
	}
	return ERROR;
}

ErrorStatus Display_SetComponent(Component_t comp, uint8_t val){
	// For components that are on/off
	if(comp <= MOTOR && val <= 1){
		// If blinkers, set the blink toggle of the components instead of the visibility
		if(comp == LEFT || comp == RIGHT || comp == HZD){
			Display_Cmd_t toggleCmd = {(char*)compStrings[comp],"en","=",1,{true},
				{
					{.num=val}
				}
			};
			Display_PutNext(toggleCmd);
			
			Display_Refresh();
		}
		else{
			Display_Cmd_t visCmd = {"vis",NULL,NULL,2,{false,true},
				{
					{.str=(char*)compStrings[comp]},
					{.num=(uint8_t)val}
				}
			};
			Display_PutNext(visCmd);
		}
	}
	// For components that have a non-boolean value
	else if(comp > MOTOR){
		Display_Cmd_t setCmd = {(char*)compStrings[comp],"val","=",1,{true},
			{
				{.num=(uint8_t)val}
			}
		};
		Display_PutNext(setCmd);
	}
	else{
		return ERROR;
	}
	return SUCCESS;
}


ErrorStatus Display_SetPage(Page_t page){
	Display_Cmd_t pgCmd = {"page",NULL,NULL,1,{true},
		{
			{.num=(uint8_t)page}
		}
	};
	Display_PutNext(pgCmd);
	return SUCCESS;
}

ErrorStatus Display_Init(){
	BSP_UART_Init(DISP_OUT);
	disp_fifo_new();
	Display_Reset();
	Display_SetPage(INFO);
	return SUCCESS;
}

ErrorStatus Display_Reset(){
	Display_Cmd_t restCmd = {"rest",NULL,NULL,0};
	Display_PutNext(restCmd);
	return SUCCESS;
}

ErrorStatus Display_SetSOC(uint8_t percent){	// Integer percentage from 0-100
	return Display_SetComponent(SOC, percent) && Display_SetComponent(SOC_BAR, percent);
}

ErrorStatus Display_SetSBPV(uint16_t mv){
	return Display_SetComponent(SUPP_BATT, mv/1000) && Display_SetComponent(SUPP_BATT_BAR, mv/1200);	// Display in volts and percentage (for the bar)
}

ErrorStatus Display_SetGear(uint8_t gear){
	Display_Cmd_t gearCmd = {"gear", "val", "=", 1, {true}, 
		{	
			{.num=(uint8_t)gear}
		}
	};
	Display_PutNext(gearCmd);
	Display_Refresh();

	return SUCCESS;
}

ErrorStatus Display_SetArray(bool state){
	return Display_SetComponent(ARRAY, (uint8_t)state);
}

ErrorStatus Display_SetMotor(bool state){
	return Display_SetComponent(MOTOR, (uint8_t)state);
}

ErrorStatus Display_SetRegenEnable(bool state){
	return Display_SetComponent(MOTOR, (uint8_t)state);
}

ErrorStatus Display_SetCruiseEnable(bool state){
	return Display_SetComponent(CRUISE, (uint8_t)state);
}

ErrorStatus Display_SetLeftBlink(bool state){
	return Display_SetComponent(LEFT, (uint8_t)state);
}

ErrorStatus Display_SetRightBlink(bool state){
	return Display_SetComponent(RIGHT, (uint8_t)state);
}

ErrorStatus Display_Fault(os_error_loc_t osErrCode, fault_bitmap_t faultCode){
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

ErrorStatus Display_Refresh(){
	// Forces nextion to reevaluate blinker and gear logic
	Display_Cmd_t refreshCmd = {"click",NULL,NULL,2,{true,true},
		{
			{.num=0},
			{.num=1}
		}
	};
	Display_PutNext(refreshCmd);
	return SUCCESS;
}