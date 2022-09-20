/** Copyright (c) 2022 UT Longhorn Racing Solar
 * @file UpdateDisplay.c
 * @brief Function implementations for the display application.
 * 
 * This contains functions relevant to modifying states of specific
 * components on our HMI design. The HMI has the ability to indicate 
 * relevant information about system status to the driver.
 * 
 * @author Ishan Deshpande (IshDeshpa)
 * @author Roie Gal (Cam0Cow)
 * @author Nathaniel Delgado (NathanielDelgado)
*/
#include "UpdateDisplay.h"
#include "Minions.h"
#include "MotorController.h"
#include <math.h>

/**
 * Creates queue for display commands.
 */
#define DISP_Q_SIZE 5

#define FIFO_TYPE Display_Cmd_t
#define FIFO_SIZE sizeof(Display_Cmd_t)*DISP_Q_SIZE
#define FIFO_NAME disp_fifo
#include "fifo.h"

disp_fifo_t msg_queue;

static OS_SEM DisplayQ_Sem4;    // counting semaphore for queue message availability
static OS_MUTEX DisplayQ_Mutex; // mutex to ensure thread safety when writing/reading to queue

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

const char* compStrings[15]= {
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

bool UpdateDisplay_Init(){
	disp_fifo_renew(&msg_queue);
	UpdateDisplay_SetPage(INFO);
	
	return true;
}

/**
 * @brief Pops the next display message from the queue and passes
 * it to the display driver. Pends on semaphore and mutex to ensure that:
 *  1) queue has messages to send (signaled by semaphore)
 *  2) queue is not currently being written to by a separate thread (mutex)
 * @returns bool: false for ERROR, true for SUCCESS
 */
static bool UpdateDisplay_PopNext(){
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

    return result && Display_Send(cmd);
}

/**
 * @brief Pops the next display message from the queue and passes
 * it to the display driver. Pends on mutex to ensure threadsafe memory access
 * and signals semaphore upon successful fifo_put.
 * @returns bool: false for ERROR, true for SUCCESS
 */
static bool UpdateDisplay_PutNext(Display_Cmd_t cmd){
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

	return success;
}

/**
 * @brief Several elements on the display do not update their
 * state until a touch/click event is triggered. This includes the
 * blinkers, gear selector, cruise control and regen braking indicator.
 * @returns bool: false for ERROR, true for SUCCESS
 */
static bool UpdateDisplay_Refresh(){
	Display_Cmd_t refreshCmd = {
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
	return UpdateDisplay_PutNext(refreshCmd);
}

/**
 * @brief Uses component enum to make assigning component values easier.
 * Differentiates between timers, variables, and components to assign values.
 * @param comp component to set value of
 * @param val value
 * @return bool: false for ERROR, true for SUCCESS
 */
static bool UpdateDisplay_SetComponent(Component_t comp, uint32_t val){
	// For components that are on/off
	if(comp <= MOTOR && val <= 1){
		// If timer components, set the toggle of the timer instead of the visibility of an item
		if(comp == LEFT || comp == RIGHT || comp == HZD){
			Display_Cmd_t toggleCmd = {
				.compOrCmd = (char*)compStrings[comp],
				.attr = "en",
				.op = "=",
				.numArgs = 1,
				.argTypes = {INT_ARG},
				{
					{.num=val}
				}
			};
			return UpdateDisplay_PutNext(toggleCmd) || UpdateDisplay_Refresh();
		}
		else{
			Display_Cmd_t visCmd = {
				.compOrCmd = "vis",
				.attr = NULL,
				.op = NULL,
				.numArgs = 2,
				.argTypes = {STR_ARG,INT_ARG},
				{
					{.str=(char*)compStrings[comp]},
					{.num=val}
				}
			};
			return UpdateDisplay_PutNext(visCmd);
		}
	}
	// For components that have a non-boolean value
	else if(comp > MOTOR){
		Display_Cmd_t setCmd = {
			.compOrCmd = (char*)compStrings[comp],
			.attr = "val",
			.op = "=",
			.numArgs = 1,
			.argTypes = {INT_ARG},
			{
				{.num=val}
			}
		};
		return UpdateDisplay_PutNext(setCmd);
	}
	else{
		return false;
	}
	return true;
}

bool UpdateDisplay_SetPage(Page_t page){
	Display_Cmd_t pgCmd = {
		.compOrCmd = "page",
		.attr = NULL,
		.op = NULL,
		.numArgs = 1,
		.argTypes = {INT_ARG},
		{
			{.num=page}
		}
	};
	return UpdateDisplay_PutNext(pgCmd);
}

/* WRAPPERS */

bool UpdateDisplay_SetSOC(uint8_t percent){	// Integer percentage from 0-100
	return UpdateDisplay_SetComponent(SOC, percent) || UpdateDisplay_Refresh();
}

bool UpdateDisplay_SetSBPV(uint32_t mv){
	return UpdateDisplay_SetComponent(SUPP_BATT, mv/100) || UpdateDisplay_Refresh();
}

bool UpdateDisplay_SetVelocity(uint32_t mphTenths){
	// units of .1 mph
	return UpdateDisplay_SetComponent(VELOCITY, mphTenths);
}

bool UpdateDisplay_SetAccel(uint8_t percent){
	return UpdateDisplay_SetComponent(ACCEL_METER, percent);
}

bool UpdateDisplay_SetArray(bool state){
	return UpdateDisplay_SetComponent(ARRAY, state);
}

bool UpdateDisplay_SetMotor(bool state){
	return UpdateDisplay_SetComponent(MOTOR, state);
}

bool UpdateDisplay_SetGear(TriState_t gear){
	return UpdateDisplay_SetComponent(GEAR, (uint32_t)gear) || UpdateDisplay_Refresh();
}

bool UpdateDisplay_SetRegenState(TriState_t state){
	return UpdateDisplay_SetComponent(REGEN_ST, (uint32_t)state) || UpdateDisplay_Refresh();
}

bool UpdateDisplay_SetCruiseState(TriState_t state){
	return UpdateDisplay_SetComponent(CRUISE_ST, (uint32_t)state) || UpdateDisplay_Refresh();
}

bool UpdateDisplay_SetLeftBlink(bool state){
	return UpdateDisplay_SetComponent(LEFT, state);
}

bool UpdateDisplay_SetRightBlink(bool state){
	return UpdateDisplay_SetComponent(RIGHT, state);
}

bool UpdateDisplay_SetHeadlight(bool state){
	return UpdateDisplay_SetComponent(HEAD, state);
}

/**
 * @brief Loops through the display queue and sends all messages
 */
void Task_UpdateDisplay(void *p_arg) {
    OS_ERR err;

    while (1) {
			if(!UpdateDisplay_PopNext()){
					FaultBitmap |= FAULT_DISPLAY;
					
					OSSemPost(&FaultState_Sem4, OS_OPT_POST_1, &err);
					assertOSError(OS_DISPLAY_LOC, err);
			}
    }
}