/**
 * Test for Display
*/

#include "Display.h"

/**
 * Enum and corresponding array for easy component selection.
 * 
 * Stolen from UpdateDisplay.c
 */
typedef enum{
	// Boolean components
	ARRAY=0,
	MOTOR,
	// Non-boolean components
	VELOCITY_DISPLAY,
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

char* test_compStrings[15]= {
	// Boolean components
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
	"faulterr"
};

// Delay; Don't know how long
void delay(void) {
	volatile int j;
	for (j = 0; j < 9999999; j++) {
		continue;
	}
}

int main(void) {
	DisplayError_t err = 0;

	err = Display_Init();
	if(err) printf("%d", err);
	delay();

	// Display the fault page
	DisplayCmd_t pgCmd = {
			.compOrCmd = "page",
			.attr = NULL,
			.op = NULL,
			.numArgs = 1,
			.argTypes = {true},
			{{.num = FAULT}}};
	err = Display_Send(pgCmd);
	if(err) printf("%d", err);
	delay();

	// Display the info page
	pgCmd = (DisplayCmd_t){
			.compOrCmd = "page",
			.attr = NULL,
			.op = NULL,
			.numArgs = 1,
			.argTypes = {true},
			{{.num = INFO}}};
	err = Display_Send(pgCmd);
	if(err) printf("%d", err);
	delay();

	// Show the array icon
	DisplayCmd_t toggleCmd = {
			.compOrCmd = "vis",
			.attr = NULL,
			.op = NULL,
			.numArgs = 2,
			.argTypes = {STR_ARG, INT_ARG},
			{{.str = test_compStrings[ARRAY]}, {.num = 1}}};
	err = Display_Send(toggleCmd);
	if(err) printf("%d", err);
	delay();

	// Don't show the array icon
	toggleCmd = (DisplayCmd_t){
			.compOrCmd = "vis",
			.attr = NULL,
			.op = NULL,
			.numArgs = 2,
			.argTypes = {STR_ARG, INT_ARG},
			{{.str = test_compStrings[ARRAY]}, {.num = 0}}};
	err = Display_Send(toggleCmd);
	if(err) printf("%d", err);
	delay();

	// Test the fault screen
	error_code_t faultCode = 0x69;
	err = Display_Error(faultCode);
	if(err) printf("%d", err);

	while(1) {}
}