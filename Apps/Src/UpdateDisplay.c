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
// #include "Minions.h"
#include <math.h>

// For fault handling
#define RESTART_THRESHOLD 3 // number of times to reset before displaying the fault screen

UpdateDisplayError_t UpdateDisplay_Init() {
	OS_ERR err;
	DisplayError_t ret = Display_SetPage(INFO);
	OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_HMSM_STRICT, &err); // Wait >215ms so errors will show on the display
	assertOSError(err);
	
	return (ret == DISPLAY_ERR_NONE) ? UPDATEDISPLAY_ERR_NONE : UPDATEDISPLAY_ERR_DRIVER;
}

/**
 * @brief Uses component enum to make assigning component values easier.
 * Differentiates between timers, variables, and components to assign values.
 * @param comp component to set value of
 * @param val value
 * @return UpdateDisplayError_t
 */
static UpdateDisplayError_t UpdateDisplay_SetComponent(Component_t comp) {
	DisplayError_t ret = DISPLAY_ERR_NONE;
	// For components that are on/off
	if (comp <= DISP_BLINK) {
		DisplayCmd_t visCmd = {
			.compOrCmd = "vis",
			.attr = NULL,
			.op = NULL,
			.numArgs = 2,
			.argTypes = {STR_ARG,INT_ARG},
			.args = {
				{.str = (char*)DISPLAY_COMP_STR[comp]},
				{.num = g_display_comp_vals[comp]}
			}
		};
		ret = Display_Send(visCmd);
	}
	// For components that have a non-boolean value but aren't the contactors
	else {
		uint32_t comp_val = 0;
		if (comp > DISP_MOTOR_PC) {
			comp_val = g_display_comp_vals[comp];
		} else { // Contactors TODO: get this updated with other contactors
			switch (comp) {
				case DISP_ARRAY_EN:
					//comp_val = Contactors_Get(); 
					break;
				case DISP_ARRAY_PC:
					comp_val = Contactors_Get(ARRAY_PRECHARGE_BYPASS_CONTACTOR);
					break;
				case DISP_MOTOR_EN:
					//comp_val = Contactors_Get();
					break;
				case DISP_MOTOR_PC:
					comp_val = Contactors_Get(MOTOR_CONTROLLER_PRECHARGE_BYPASS_CONTACTOR);
					break;
				default:
				break;
			}
		}

		DisplayCmd_t setCmd = {
			.compOrCmd = (char*)DISPLAY_COMP_STR[comp],
			.attr = "val",
			.op = "=",
			.numArgs = 1,
			.argTypes = {INT_ARG},
			.args = {{.num = comp_val}}
		};
		ret = Display_Send(setCmd);
	}
	
	return (ret == DISPLAY_ERR_NONE) ? UPDATEDISPLAY_ERR_NONE : UPDATEDISPLAY_ERR_DRIVER;
}

/* WRAPPERS */
UpdateDisplayError_t UpdateDisplay_SetSOC(uint32_t percent) {	// Integer percentage from 0-100
	if (percent > 100) {
		g_display_comp_vals[DISP_SOC] = 123;
		return UPDATEDISPLAY_ERR_PARSE_COMP;
	}

	g_display_comp_vals[DISP_SOC] = percent;
	return UPDATEDISPLAY_ERR_NONE;
}

UpdateDisplayError_t UpdateDisplay_SetSBPV(uint32_t mv) {
	g_display_comp_vals[DISP_SUPP_BATT] = mv;
	return UPDATEDISPLAY_ERR_NONE;
}

UpdateDisplayError_t UpdateDisplay_SetVelocity(uint32_t mphTenths) {
	g_display_comp_vals[DISP_VELOCITY] = mphTenths;
	return UPDATEDISPLAY_ERR_NONE;
}

UpdateDisplayError_t UpdateDisplay_SetAccel(uint8_t percent) {
	g_display_comp_vals[DISP_ACCEL_METER] = (uint32_t)((percent > 100) ? 100 : percent);
	return UPDATEDISPLAY_ERR_NONE;
}

UpdateDisplayError_t UpdateDisplay_SetBlink(bool state) {
	g_display_comp_vals[DISP_BLINK] = (uint32_t)(state);
	return UPDATEDISPLAY_ERR_NONE;
}

UpdateDisplayError_t UpdateDisplay_SetGear(TriState_t gear) {
	g_display_comp_vals[DISP_GEAR] = (uint32_t)(gear);
	return UPDATEDISPLAY_ERR_NONE;
}

UpdateDisplayError_t UpdateDisplay_SetRegenState(TriState_t state) {
	g_display_comp_vals[DISP_REGEN_ST] = (uint32_t)(state);
	return UPDATEDISPLAY_ERR_NONE;
}

UpdateDisplayError_t UpdateDisplay_SetCruiseState(TriState_t state) {
	g_display_comp_vals[DISP_CRUISE_ST] = (uint32_t)(state);
	return UPDATEDISPLAY_ERR_NONE;
}

UpdateDisplayError_t UpdateDisplay_SetBattVoltage(uint32_t mv) {
	g_display_comp_vals[DISP_PACK_VOLTAGE] = (mv / 100); // mv to tenths of a volt
	return UPDATEDISPLAY_ERR_NONE;
}

UpdateDisplayError_t UpdateDisplay_SetBattTemperature(uint32_t val) {
	g_display_comp_vals[DISP_PACK_TEMP] = (val / 100);
	return UPDATEDISPLAY_ERR_NONE;
}

UpdateDisplayError_t UpdateDisplay_SetBattCurrent(int32_t val) {
	bool is_neg = (val < 0);
	g_display_comp_vals[DISP_PACK_CURR_SIGN] = (uint32_t)(is_neg);
	g_display_comp_vals[DISP_PACK_CURRENT]	 = (uint32_t)((is_neg ? -val : val) / 100);
	return UPDATEDISPLAY_ERR_NONE;
}

UpdateDisplayError_t UpdateDisplay_SetMCVoltage(uint32_t volts) {
	g_display_comp_vals[DISP_MC_BUS_VOLTAGE] = volts;
	return UPDATEDISPLAY_ERR_NONE;
}

UpdateDisplayError_t UpdateDisplay_SetMCCurrent(int32_t val) {
	bool is_neg = (val < 0);
	g_display_comp_vals[DISP_MC_CURR_SIGN]   = (uint32_t)(is_neg);
	g_display_comp_vals[DISP_MC_BUS_CURRENT] = (uint32_t)(is_neg ? -val : val);
	return UPDATEDISPLAY_ERR_NONE;
}

UpdateDisplayError_t UpdateDisplay_SetBrake(bool state) {
	g_display_comp_vals[DISP_BRAKE] = (uint32_t)(state);
	return UPDATEDISPLAY_ERR_NONE;
}

UpdateDisplayError_t UpdateDisplay_SetHeartbeat(bool val) {
	g_display_comp_vals[DISP_HEARTBEAT] = (uint32_t)(val);
	return UPDATEDISPLAY_ERR_NONE;
}

UpdateDisplayError_t UpdateDisplay_SetHeatSinkTemp(uint32_t val) {
	g_display_comp_vals[DISP_HEAT_SINK_TEMP] = val;
	return UPDATEDISPLAY_ERR_NONE;
}


/**
 * @brief Loops through the display queue and sends all messages
 */
void Task_UpdateDisplay(void *p_arg) {
	OS_ERR err;
	while (1) {
		for (Component_t comp = 0; comp <= DISP_GEAR; comp++) {
			if (comp != DISP_REGEN_ST && comp != DISP_CRUISE_ST) {
				assertUpdateDisplayError(UpdateDisplay_SetComponent(comp));
			}
		}
		assertUpdateDisplayError(UpdateDisplay_SetHeartbeat(g_display_comp_vals[DISP_HEARTBEAT] ? 0 : 1));

		assertUpdateDisplayError(
			(Display_Refresh() == DISPLAY_ERR_NONE) 
				? UPDATEDISPLAY_ERR_NONE
				: UPDATEDISPLAY_ERR_DRIVER
		);

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
void handler_UpdateDisplay_Restart() {
    Display_Reset(); // Try resetting to fix the display error
}

/**
 * @brief Check for a display error and assert it if it exists.
 * Stores the error code, calls the main assertion function 
 * and runs a callback function as a handler to restart the display and clear the queue.
 * @param   err variable with display error codes
 */
void assertUpdateDisplayError(UpdateDisplayError_t err) {
	Error_UpdateDisplay = (error_code_t)err; // Store the error code for inspection

	if (err == UPDATEDISPLAY_ERR_NONE) return; // No error, return

    // Otherwise try resetting the display using the restart callback
    throwTaskError(Error_UpdateDisplay, handler_UpdateDisplay_Restart,OPT_NO_LOCK_SCHED, OPT_RECOV);

    Error_UpdateDisplay = UPDATEDISPLAY_ERR_NONE; // Clear the error after handling it
}
