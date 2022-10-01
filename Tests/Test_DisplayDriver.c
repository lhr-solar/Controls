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
// #include "MotorController.h"
// #include "Contactors.h"
#include "Display.h"

// Stolen from UpdateDisplay.c
/**
 * Enum and corresponding array for easy component selection.
 */
typedef enum{
	// Boolean components
	LEFT=0,
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

static char* compStrings[15]= {
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
	"faulterr"
};

// Delay; Don't know how long
void delay(void){
    for(int i = 0; i < 999999; i++){
        continue;
    }
}

int main(){
	Display_Error_t err;

    err = Display_Init();
	assertDisplayError(err);
    delay();
    
    // Test if the reset works (should show the start screen again)
    err = Display_Reset();
	assertDisplayError(err);
    delay();

    // Display the fault page
    Display_Cmd_t pgCmd = {
		.compOrCmd = "page",
		.attr = NULL,
		.op = NULL,
		.numArgs = 1,
		.argTypes = {true},
		{
			{.num=FAULT}
		}
	};
    err = Display_Send(pgCmd);
	assertDisplayError(err);
    delay();

    // Display the info page
    pgCmd = (Display_Cmd_t){
		.compOrCmd = "page",
		.attr = NULL,
		.op = NULL,
		.numArgs = 1,
		.argTypes = {true},
		{
			{.num=INFO}
		}
	};
    err = Display_Send(pgCmd);
	assertDisplayError(err);
    delay();

    // Show the array icon
    Display_Cmd_t toggleCmd = {
        .compOrCmd = (char*)compStrings[ARRAY],
        .attr = "en",
        .op = "=",
        .numArgs = 1,
        .argTypes = {true},
        {
            {.num=1}
        }
    };
    err = Display_Send(toggleCmd);
	assertDisplayError(err);
    delay();

    // Don't show the array icon
    toggleCmd = (Display_Cmd_t){
        .compOrCmd = (char*)compStrings[ARRAY],
        .attr = "en",
        .op = "=",
        .numArgs = 1,
        .argTypes = {true},
        {
            {.num=0}
        }
    };
    err = Display_Send(toggleCmd);
	assertDisplayError(err);
    delay();

    // Test the fault screen
    os_error_loc_t osErrCode = 0x0420;
    fault_bitmap_t faultCode = 0x69;
    err = Display_Fault(osErrCode, faultCode);
	assertDisplayError(err);
    
    while(1){
        
    }
}