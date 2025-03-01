/**
 * @file Test_DisplayDriver.c
 * @author Nathaniel Delgado (nathaniel.delgado@utexas.edu)
 * @brief Tests the driver code for display
 * @version 0.1
 * @date 2022-10-00
 *
 * @copyright Copyright (c) 2022
 *
 */

// #include "common.h"
// #include "config.h"
// #include "os.h"
#include "Tasks.h"
// #include "Display.h"
// #include "bsp.h"
// #include "Contactors.h"
#include "Display.h"
#include "bsp.h"   // for writing to UART
#define DISP_OUT UART_3
// static const char *TERMINATOR = "\xff\xff\xff";


// Stolen from UpdateDisplay.c
/**
 * Enum and corresponding array for easy component selection.
 */
typedef enum
{
	// Boolean components
	LEFT = 0,
	HEAD,
	RIGHT,
	HZD,
	ARRAY,
	MOTOR,
	// Non-boolean components
	VELOCITY,
	ACCEL_METER,
	SOC,
	SUPP_BATT,
	CRUISE_ST,
	REGEN_ST,
	GEAR,
	// Fault code components
	OS_CODE,
	FAULT_CODE
} Component_t;

static char *compStrings[15] = {
		// Boolean components
		"ltime",
		"head",
		"rtime",
		"hzd",
		"arr",
		"mot",
		// Non-boolean components
		"vel",
		"accel",
		"soc",
		"supp",
		"cruiseSt",
		"rbsSt",
		"gear",
		// Fault code components
		"oserr",
		"faulterr"};

// Delay; Don't know how long
void delay(void)
{
	volatile int j;
	for (j = 0; j < 9999999; j++)
	{
		continue;
	}
}

int main()
{
	DisplayError_t err;

	err = Display_Init();
	//assertDisplayError(err);
	DisplayCmd_t setCmd = {
		.compOrCmd = (char*)"faulterr",
		.attr = "val",
		.op = "=",
		.numArgs = 1,
		.argTypes = {INT_ARG},
		{
			{.num="1200"}
		}
	};
	Display_Send(setCmd);
	delay();
	Display_Send(setCmd);



	// char* faultMsg = "";
	// faultMsg = "vel.val=12";
    // BSP_UART_Write(DISP_OUT, faultMsg, strlen(faultMsg));
	// BSP_UART_Write(DISP_OUT, (char *)TERMINATOR, strlen(TERMINATOR));

	// Display the fault page
	DisplayCmd_t pgCmd = {
			.compOrCmd = "page",
			.attr = NULL,
			.op = NULL,
			.numArgs = 1,
			.argTypes = {true},
			{{.num = FAULT}}};
	err = Display_Send(pgCmd);
	//assertDisplayError(err);
	// BSP_UART_Write(DISP_OUT, faultMsg, strlen(faultMsg));
	// BSP_UART_Write(DISP_OUT, (char *)TERMINATOR, strlen(TERMINATOR));
	Display_Send(setCmd);
	delay();

	// BSP_UART_Write(DISP_OUT, faultMsg, strlen(faultMsg));
	// BSP_UART_Write(DISP_OUT, (char *)TERMINATOR, strlen(TERMINATOR));
	Display_Send(setCmd);


	// Display the info page
	pgCmd = (DisplayCmd_t){
			.compOrCmd = "page",
			.attr = NULL,
			.op = NULL,
			.numArgs = 1,
			.argTypes = {true},
			{{.num = INFO}}};
	err = Display_Send(pgCmd);
	Display_Send(setCmd);
	//assertDisplayError(err);
	delay();
	Display_Send(setCmd);


	// Show the array icon
	DisplayCmd_t toggleCmd = {
			.compOrCmd = "vis",
			.attr = NULL,
			.op = NULL,
			.numArgs = 2,
			.argTypes = {STR_ARG, INT_ARG},
			{{.str = compStrings[ARRAY]}, {.num = 1}}};
	err = Display_Send(toggleCmd);
	//assertDisplayError(err);
	delay();
	Display_Send(setCmd);


	// Don't show the array icon
	toggleCmd = (DisplayCmd_t){
			.compOrCmd = "vis",
			.attr = NULL,
			.op = NULL,
			.numArgs = 2,
			.argTypes = {STR_ARG, INT_ARG},
			{{.str = compStrings[ARRAY]}, {.num = 0}}};
	err = Display_Send(toggleCmd);
	//assertDisplayError(err);
	delay();
	//assertDisplayError(err);
	Display_Send(setCmd);


	//Test the fault screen
	error_code_t faultCode = 0x69;
	err = Display_Error(faultCode);
	printf("%x\n", err);
	Display_Send(setCmd);



	while (1)
	{
	}
}