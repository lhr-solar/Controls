// #include "common.h"
// #include "config.h"
#include "os.h"
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
    if(Display_Init() != SUCCESS){
        return 0;
    }
    delay();
    
    // Test if the reset works (should show the start screen agian)
    Display_Reset();
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
    Display_Send(pgCmd);
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
    Display_Send(pgCmd);
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
    Display_Send(toggleCmd);
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
    Display_Send(toggleCmd);
    delay();

    // Test the fault screen
    os_error_loc_t osErrCode = 0x0420;
    fault_bitmap_t faultCode = 0x69;
    Display_Fault(osErrCode, faultCode);
    
    while(1){
        
    }
}