/**
 * @copyright Copyright (c) 2018-2023 UT Longhorn Racing Solar
 * @file UpdateDisplay.c
 * @brief Function implementations for the display application.
 * 
 * This contains functions relevant to modifying states of specific
 * components on our HMI design. The HMI has the ability to indicate 
 * relevant information about system status to the driver.
 * 
 */

#include "UpdateDisplay.h"
#include "Minions.h"
#include <math.h>

// For fault handling
#define RESTART_THRESHOLD 3 // number of times to reset before displaying the fault screen

/**
 * Function prototypes
*/
// check for and assert errors in UpdateDisplay
static void assertUpdateDisplayError(UpdateDisplayError_t err);

/**
 * Enum and corresponding array for easy component selection.
 */
typedef enum{
	// Boolean components
	ARRAY=0,
    HEARTBEAT,
    PACK_CURR_SIGN,
	MOTOR,
	// Non-boolean components
	VELOCITY,
	ACCEL_METER,
	SOC,
	SUPP_BATT,
	CRUISE_ST,
	REGEN_ST,
    PACK_VOLTAGE,
    PACK_CURRENT,
    PACK_TEMP,
	GEAR,
	// Fault code components
	OS_CODE,
	FAULT_CODE,
    NUM_COMPONENTS
} Component_t;

static uint32_t componentVals[NUM_COMPONENTS] = {0};

const char* compStrings[16]= {
	// Boolean components
	"arr",
    "hb",
    "cs",
	"mot",
	// Non-boolean components
	"vel",
	"accel",
	"soc",
	"supp",
	"cruiseSt",
	"rbsSt",
    "pv",
    "pc",
    "pt",
	"gear",
	// Fault code components
	"oserr",
	"faulterr"
};

UpdateDisplayError_t UpdateDisplay_Init(){
	OS_ERR err;

	UpdateDisplayError_t ret = UpdateDisplay_SetPage(INFO);
    OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT, &err); // Wait >215ms so errors will show on the display
    assertOSError(err);
	
    return ret;
}

/**
 * @brief Several elements on the display do not update their
 * state until a touch/click event is triggered. This includes the
 * blinkers, gear selector, cruise control and regen braking indicator.
 * @returns UpdateDisplayError_t
 */
static UpdateDisplayError_t UpdateDisplay_Refresh(){
	DisplayCmd_t refreshCmd = {
		.compOrCmd = "click",
		.attr = NULL,
		.op = NULL,
		.numArgs = 2,
		.argTypes = {INT_ARG,INT_ARG},
		{
			{.num=0},
			{.num=1}
		}
	};

	Display_Send(refreshCmd);
	return UPDATEDISPLAY_ERR_NONE;
}

/**
 * @brief Uses component enum to make assigning component values easier.
 * Differentiates between timers, variables, and components to assign values.
 * @param comp component to set value of
 * @param val value
 * @return UpdateDisplayError_t
 */
static UpdateDisplayError_t UpdateDisplay_SetComponent(Component_t comp){
	UpdateDisplayError_t ret = UPDATEDISPLAY_ERR_NONE;
	
	// For components that are on/off
	if(comp <= MOTOR && componentVals[comp] <= 1){
		DisplayCmd_t visCmd = {
			.compOrCmd = "vis",
			.attr = NULL,
			.op = NULL,
			.numArgs = 2,
			.argTypes = {STR_ARG,INT_ARG},
			{
				{.str=(char*)compStrings[comp]},
				{.num=componentVals[comp]}
			}
		};
		
		ret = Display_Send(visCmd);
		return ret;
	}
	// For components that have a non-boolean value
	else if(comp > MOTOR){
		DisplayCmd_t setCmd = {
			.compOrCmd = (char*)compStrings[comp],
			.attr = "val",
			.op = "=",
			.numArgs = 1,
			.argTypes = {INT_ARG},
			{
				{.num=componentVals[comp]}
			}
		};

		ret = Display_Send(setCmd);
		return ret;
	}
	else{
		assertUpdateDisplayError(UPDATEDISPLAY_ERR_PARSE_COMP);
		return UPDATEDISPLAY_ERR_PARSE_COMP;
	}
	return UPDATEDISPLAY_ERR_NONE;
}

UpdateDisplayError_t UpdateDisplay_SetPage(Page_t page){
	DisplayCmd_t pgCmd = {
		.compOrCmd = "page",
		.attr = NULL,
		.op = NULL,
		.numArgs = 1,
		.argTypes = {INT_ARG},
		{
			{.num=page}
		}
	};

    Display_Send(pgCmd);
	
	return UPDATEDISPLAY_ERR_NONE;
}

/* WRAPPERS */
UpdateDisplayError_t UpdateDisplay_SetSOC(uint32_t percent){	// Integer percentage from 0-100
	componentVals[SOC] = (percent);
    
    return UPDATEDISPLAY_ERR_NONE;
}

UpdateDisplayError_t UpdateDisplay_SetSBPV(uint32_t mv){
	componentVals[SUPP_BATT] = mv;
    
    return UPDATEDISPLAY_ERR_NONE;
}

UpdateDisplayError_t UpdateDisplay_SetVelocity(uint32_t mphTenths){
	componentVals[VELOCITY] = mphTenths;

    return UPDATEDISPLAY_ERR_NONE;
}

UpdateDisplayError_t UpdateDisplay_SetAccel(uint8_t percent){
    componentVals[ACCEL_METER] = percent;
    
    return UPDATEDISPLAY_ERR_NONE;
}

UpdateDisplayError_t UpdateDisplay_SetArray(bool state){
	componentVals[ARRAY] = state;
    
    return UPDATEDISPLAY_ERR_NONE;
}

UpdateDisplayError_t UpdateDisplay_SetMotor(bool state){
	componentVals[MOTOR] = state;

    return UPDATEDISPLAY_ERR_NONE;
}

UpdateDisplayError_t UpdateDisplay_SetGear(TriState_t gear){
    componentVals[GEAR] = gear;
	
    return UPDATEDISPLAY_ERR_NONE;
}

UpdateDisplayError_t UpdateDisplay_SetRegenState(TriState_t state){
	componentVals[REGEN_ST] = state;

	return UPDATEDISPLAY_ERR_NONE;
}

UpdateDisplayError_t UpdateDisplay_SetCruiseState(TriState_t state){
    componentVals[CRUISE_ST] = state;
	
    return UPDATEDISPLAY_ERR_NONE;
}

UpdateDisplayError_t UpdateDisplay_SetBattVoltage(uint32_t mv){
    componentVals[PACK_VOLTAGE] = (mv/100); // mv to tenths of a volt
	
    return UPDATEDISPLAY_ERR_NONE;
}

UpdateDisplayError_t UpdateDisplay_SetBattTemperature(uint32_t val){
	componentVals[PACK_TEMP] = (val/100);

	return UPDATEDISPLAY_ERR_NONE;
}

UpdateDisplayError_t UpdateDisplay_SetBattCurrent(int32_t val){
    componentVals[PACK_CURRENT] = (((uint32_t)((val<0)?-val:val))/100);
    componentVals[PACK_CURR_SIGN] = (val < 0)?1:0;
    return UPDATEDISPLAY_ERR_NONE;
}

UpdateDisplayError_t UpdateDisplay_SetHeartbeat(uint32_t val){
    componentVals[HEARTBEAT] = val;

    return UPDATEDISPLAY_ERR_NONE;
}


/**
 * @brief Loops through the display queue and sends all messages
 */
void Task_UpdateDisplay(void *p_arg) {
    OS_ERR err;
    while (1) {
		for(Component_t comp = ARRAY; comp <= GEAR; comp++){
            UpdateDisplay_SetComponent(comp);
        }

        componentVals[HEARTBEAT] = (componentVals[HEARTBEAT]?0:1);

        UpdateDisplay_Refresh();

        // Delay of 250 ms
        OSTimeDlyHMSM(0, 0, 0, 250, OS_OPT_TIME_HMSM_STRICT, &err);
        assertOSError(err);
    }
}

/**
 * Error handler functions
 * Passed as callback functions to the main throwTaskError function by assertUpdateDisplayError
*/

/**
 * @brief A handler callback function run by the main throwTaskError function
 * used if we haven't reached the restart limit and encounter an error
 */ 
static void handler_UpdateDisplay_Restart() {
    Display_Reset(); // Try resetting to fix the display error
}

/**
 * @brief Check for a display error and assert it if it exists.
 * Stores the error code, calls the main assertion function 
 * and runs a callback function as a handler to restart the display and clear the queue.
 * @param   err variable with display error codes
 */
 static void assertUpdateDisplayError(UpdateDisplayError_t err){
	Error_UpdateDisplay = (error_code_t)err; // Store the error code for inspection

	if (err == UPDATEDISPLAY_ERR_NONE) return; // No error, return

    // Otherwise try resetting the display using the restart callback
    throwTaskError(Error_UpdateDisplay, handler_UpdateDisplay_Restart,OPT_NO_LOCK_SCHED, OPT_RECOV);

    Error_UpdateDisplay = UPDATEDISPLAY_ERR_NONE; // Clear the error after handling it
}
