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

	while (1)
	{
		Display_SetPage(INFO);
		// assertDisplayError(err);
		delay();

		// Show the brake icon
		DisplayCmd_t toggleCmd = {
			.compOrCmd = "vis",
			.attr = NULL,
			.op = NULL,
			.numArgs = 2,
			.argTypes = {STR_ARG, INT_ARG},
			.args = {{.str = compStrings[DISP_BRAKE]}, {.num = 1}}
		};

		err = Display_Send(toggleCmd);
		delay();

		// show the blinker icon
		toggleCmd = (DisplayCmd_t){
			.compOrCmd = "vis",
			.attr = NULL,
			.op = NULL,
			.numArgs = 2,
			.argTypes = {STR_ARG, INT_ARG},
			.args = {{.str = compStrings[DISP_BLINK]}, {.num = 1}}
		};
		err = Display_Send(toggleCmd);
		delay();

		// Hide the brake icon
		toggleCmd = (DisplayCmd_t){
			.compOrCmd = "vis",
			.attr = NULL,
			.op = NULL,
			.numArgs = 2,
			.argTypes = {STR_ARG, INT_ARG},
			.args = {{.str = compStrings[DISP_BRAKE]}, {.num = 0}}};
		err = Display_Send(toggleCmd);
		delay();

		// Hide the blinker icon
		toggleCmd = (DisplayCmd_t){
			.compOrCmd = "vis",
			.attr = NULL,
			.op = NULL,
			.numArgs = 2,
			.argTypes = {STR_ARG, INT_ARG},
			.args = {{.str = compStrings[DISP_BLINK]}, {.num = 0}}};
		err = Display_Send(toggleCmd);
		delay();

		DisplayCmd_t setCmd = (DisplayCmd_t) {
			.compOrCmd = (char*)DISPLAY_COMP_STR[DISP_ACCEL_METER],
			.attr = "val",
			.op = "=",
			.numArgs = 1,
			.argTypes = {INT_ARG},
			.args = {{.num = 50}}
		};
		err = Display_Send(setCmd);
		delay();

		setCmd = (DisplayCmd_t) {
			.compOrCmd = (char*)DISPLAY_COMP_STR[DISP_SOC],
			.attr = "val",
			.op = "=",
			.numArgs = 1,
			.argTypes = {INT_ARG},
			.args = {{.num = 60}}
		};
		err = Display_Send(setCmd);
		delay();

		setCmd = (DisplayCmd_t) {
			.compOrCmd = (char*)DISPLAY_COMP_STR[DISP_VELOCITY],
			.attr = "val",
			.op = "=",
			.numArgs = 1,
			.argTypes = {INT_ARG},
			.args = {{.num = 100}}
		};
		err = Display_Send(setCmd);
		delay();

		setCmd = (DisplayCmd_t) {
			.compOrCmd = (char*)DISPLAY_COMP_STR[DISP_VELOCITY],
			.attr = "val",
			.op = "=",
			.numArgs = 1,
			.argTypes = {INT_ARG},
			.args = {{.num = 100}}
		};
		err = Display_Send(setCmd);
		delay();

		// Test the fault screens
		printf("%d", err);

		delay();
		delay();
		assertOSError(OS_ERR_X);

		delay();
		Display_Error();
		delay();

		// Uncomment one of these at a time (these will lock the scheduler)
		// assertReadCarCANError(READCARCAN_ERR_BPS_TRIP);
		// assertTritiumError(T_SOFTWARE_OVER_CURRENT_ERR);
		assertUpdateDisplayError(UPDATEDISPLAY_ERR_DRIVER);


		delay();
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