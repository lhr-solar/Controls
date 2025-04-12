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
#include "ReadTritium.h"
#include "UpdateDisplay.h"
#include "ReadCarCAN.h"
#include "bsp.h" // for writing to UART
// static const char *TERMINATOR = "\xff\xff\xff";

static OS_TCB Task1_TCB;
static CPU_STK Task1_Stk[DEFAULT_STACK_SIZE];

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
	for (j = 0; j < 2000000; j++)
		continue;
}

void Task1()
{
	DisplayError_t err;

	err = Display_Init();
	// assertDisplayError(err);
	DisplayCmd_t setCmd = {
		.compOrCmd = (char *)"faulterr",
		.attr = "val",
		.op = "=",
		.numArgs = 1,
		.argTypes = {STR_ARG},
		{{.str = "1200"}}};
	Display_Send(setCmd);
	delay();
	Display_Send(setCmd);

	// Display the fault page
	DisplayCmd_t pgCmd = {
		.compOrCmd = "page",
		.attr = NULL,
		.op = NULL,
		.numArgs = 1,
		.argTypes = {true},
		{{.num = FAULT}}};

	while (1)
	{
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
		// assertDisplayError(err);
		delay();
		Display_Send(setCmd);

		

		// Show the array icon
		DisplayCmd_t toggleCmd = {
			.compOrCmd = "vis",
			.attr = NULL,
			.op = NULL,
			.numArgs = 2,
			.argTypes = {STR_ARG, INT_ARG},
			{{.str = compStrings[4]}, {.num = 1}}};

		err = Display_Send(toggleCmd);
		delay();

		// show the mot icon
		toggleCmd = (DisplayCmd_t){
			.compOrCmd = "vis",
			.attr = NULL,
			.op = NULL,
			.numArgs = 2,
			.argTypes = {STR_ARG, INT_ARG},
			{{.str = compStrings[5]}, {.num = 1}}};
		err = Display_Send(toggleCmd);
		delay();

		// Don't show the array icon
		toggleCmd = (DisplayCmd_t){
			.compOrCmd = "vis",
			.attr = NULL,
			.op = NULL,
			.numArgs = 2,
			.argTypes = {STR_ARG, INT_ARG},
			{{.str = compStrings[4]}, {.num = 0}}};
		err = Display_Send(toggleCmd);
		delay();

		// Don't show the mot icon
		toggleCmd = (DisplayCmd_t){
			.compOrCmd = "vis",
			.attr = NULL,
			.op = NULL,
			.numArgs = 2,
			.argTypes = {STR_ARG, INT_ARG},
			{{.str = compStrings[5]}, {.num = 0}}};
		err = Display_Send(toggleCmd);
		delay();

		// Don't show the mot icon
		toggleCmd = (DisplayCmd_t){
			.compOrCmd = "vis",
			.attr = NULL,
			.op = NULL,
			.numArgs = 2,
			.argTypes = {STR_ARG, INT_ARG},
			{{.str = compStrings[5]}, {.num = 0}}};
		err = Display_Send(toggleCmd);
		delay();


		// Test the fault screen
		//  error_code_t faultCode = 0x69;

		// delay();
		//  delay();
		//  assertOSError(OS_ERR_X);
		//  delay();
		//  Display_Error();

		// assertReadCarCANError(READCARCAN_ERR_BPS_TRIP);
		// Display_Error();

		// assertTritiumError(T_SOFTWARE_OVER_CURRENT_ERR);
		// delay();
		// Display_Error();

		// err = Display_Error();
		printf("%x\n", err);
		// Display_Send(setCmd);
	}

	// while (1) {;}
}

int main()
{
	OS_ERR err;
	OSInit(&err);

	if (err != OS_ERR_NONE)
	{
		printf("OS error code %d\n", err);
	}
	OSTaskCreate(
		(OS_TCB *)&Task1_TCB,
		(CPU_CHAR *)"Task1",
		(OS_TASK_PTR)Task1,
		(void *)NULL,
		(OS_PRIO)13,
		(CPU_STK *)Task1_Stk,
		(CPU_STK_SIZE)DEFAULT_STACK_SIZE / 10,
		(CPU_STK_SIZE)DEFAULT_STACK_SIZE,
		(OS_MSG_QTY)0,
		(OS_TICK)NULL,
		(void *)NULL,
		(OS_OPT)(OS_OPT_TASK_STK_CLR),
		(OS_ERR *)&err);
	assertOSError(err);

	OSStart(&err);
	assertOSError(err);

	while (1)
	{
	};
}