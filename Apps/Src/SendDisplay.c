/** Copyright (c) 2022 UT Longhorn Racing Solar
 * @file SendDisplay.c
 * @brief Function implementations for the display application.
 * 
 * This contains functions relevant to modifying states of specific
 * components on our HMI design. The HMI has the ability to indicate 
 * relevant information about system status to the driver.
 * 
 * @author Ishan Deshpande (IshDeshpa)
 * @author Roie Gal (Cam0Cow)
*/
#include "SendDisplay.h"
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

/**
 * @brief Pops the next display message from the queue and passes
 * it to the display driver. Pends on semaphore and mutex to ensure that:
 *  1) queue has messages to send (signaled by semaphore)
 *  2) queue is not currently being written to by a separate thread (mutex)
 * @returns ErrorStatus: ERROR or SUCCESS
 */
static ErrorStatus Display_PopNext(){
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

    Display_Send(cmd);
}

/**
 * @brief Pops the next display message from the queue and passes
 * it to the display driver. Pends on mutex to ensure threadsafe memory access
 * and signals semaphore upon successful fifo_put.
 * @returns ErrorStatus: ERROR or SUCCESS
 */
static ErrorStatus Display_PutNext(Display_Cmd_t cmd){
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

/**
 * @brief Several elements on the display do not update their
 * state until a touch/click event is triggered. This includes the
 * blinkers, gear selector, cruise control and regen braking indicator.
 * @returns ErrorStatus: ERROR or SUCCESS
 */
static ErrorStatus Display_Refresh(){
	Display_Cmd_t refreshCmd = {
		.compOrCmd = "click",
		.attr = NULL,
		.op = NULL,
		.numArgs = 2,
		.argTypes = {true,true},
		{
			{.num=0},
			{.num=1}
		}
	};
	return Display_PutNext(refreshCmd);
}

/**
 * @brief Uses component enum to make assigning component values easier.
 * Differentiates between timers, variables, and components to assign values.
 * @param comp component to set value of
 * @param val value
 */
static ErrorStatus Display_SetComponent(Component_t comp, uint32_t val){
	// For components that are on/off
	if(comp <= MOTOR && val <= 1){
		// If timer components, set the toggle of the timer instead of the visibility of an item
		if(comp == LEFT || comp == RIGHT || comp == HZD){
			Display_Cmd_t toggleCmd = {
				.compOrCmd = (char*)compStrings[comp],
				.attr = "en",
				.op = "=",
				.numArgs = 1,
				.argTypes = {true},
				{
					{.num=val}
				}
			};
			return Display_PutNext(toggleCmd) && Display_Refresh();
		}
		else{
			Display_Cmd_t visCmd = {
				.compOrCmd = "vis",
				.attr = NULL,
				.op = NULL,
				.numArgs = 2,
				.argTypes = {false,true},
				{
					{.str=(char*)compStrings[comp]},
					{.num=val}
				}
			};
			return Display_PutNext(visCmd);
		}
	}
	// For components that have a non-boolean value
	else if(comp > MOTOR){
		Display_Cmd_t setCmd = {
			.compOrCmd = (char*)compStrings[comp],
			.attr = "val",
			.op = "=",
			.numArgs = 1,
			.argTypes = {true},
			{
				{.num=val}
			}
		};
		return Display_PutNext(setCmd);
	}
	else{
		return ERROR;
	}
	return SUCCESS;
}

ErrorStatus Display_SetPage(Page_t page){
	Display_Cmd_t pgCmd = {
		.compOrCmd = "page",
		.attr = NULL,
		.op = NULL,
		.numArgs = 1,
		.argTypes = {true},
		{
			{.num=page}
		}
	};
	return Display_PutNext(pgCmd);
}

/* WRAPPERS */

ErrorStatus Display_SetSOC(uint8_t percent){	// Integer percentage from 0-100
	return Display_SetComponent(SOC, percent) && Display_Refresh();
}

ErrorStatus Display_SetSBPV(uint32_t mv){
	return Display_SetComponent(SUPP_BATT, mv/100) && Display_Refresh();
}

ErrorStatus Display_SetVelocity(uint32_t mphTenths){
	// units of .1 mph
	return Display_SetComponent(VELOCITY, mphTenths);
}

ErrorStatus Display_SetAccel(uint8_t percent){
	return Display_SetComponent(ACCEL_METER, percent);
}

ErrorStatus Display_SetArray(bool state){
	return Display_SetComponent(ARRAY, state);
}

ErrorStatus Display_SetMotor(bool state){
	return Display_SetComponent(MOTOR, state);
}

ErrorStatus Display_SetGear(TriState_t gear){
	// Even though this isn't a three stage component, i'm still using the same enum because i'm lazy
	return Display_SetComponent(GEAR, (uint32_t)gear) && Display_Refresh();
}

ErrorStatus Display_SetRegenState(TriState_t state){
	return Display_SetComponent(REGEN_ST, (uint32_t)state) && Display_Refresh();
}

ErrorStatus Display_SetCruiseState(TriState_t state){
	return Display_SetComponent(CRUISE_ST, (uint32_t)state) && Display_Refresh();
}

ErrorStatus Display_SetLeftBlink(bool state){
	return Display_SetComponent(LEFT, state);
}

ErrorStatus Display_SetRightBlink(bool state){
	return Display_SetComponent(RIGHT, state);
}

ErrorStatus Display_SetHeadlight(bool state){
	return Display_SetComponent(HEAD, state);
}

/**
 * @brief Loops through the display queue and sends all messages
 */
void Task_SendDisplay(void *p_arg) {
    OS_ERR err;

    disp_fifo_new();
    Display_SetPage(INFO);

    while (1) {
        if(Display_SendNext() == ERROR){
            FaultBitmap |= FAULT_DISPLAY;
            
            OSSemPost(&FaultState_Sem4, OS_OPT_POST_1, &err);
        }
        
        //OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_NON_STRICT, &err); // Update screen at roughly 10 fps
        //assertOSError(OS_DISPLAY_LOC, err);
    }
}