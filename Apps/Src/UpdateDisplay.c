/** 
 * @copyright Copyright (c) 2022 UT Longhorn Racing Solar
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

/** @addtogroup UpdateDisplay 
 * @{ */

#include "UpdateDisplay.h"
#include "Minions.h"
#include <math.h>

/**
 * Creates queue for display commands.
 */
#define DISP_Q_SIZE 10

#define FIFO_TYPE DisplayCmd_t
#define FIFO_SIZE DISP_Q_SIZE
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
	ARRAY=0,
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
 * @brief Error handler for any UpdateDisplay errors. Call this after any display application function.
 */
static void assertUpdateDisplayError(UpdateDisplayError_t err){
	OS_ERR os_err;

	if(err != UPDATEDISPLAY_ERR_NONE){
		FaultBitmap |= FAULT_DISPLAY;

		OSSemPost(&FaultState_Sem4, OS_OPT_POST_1, &os_err);
		assertOSError(OS_DISPLAY_LOC, os_err);
	}
}

UpdateDisplayError_t UpdateDisplay_Init(){
	OS_ERR err;
	disp_fifo_renew(&msg_queue);
	OSMutexCreate(&DisplayQ_Mutex, "Display mutex", &err);
	assertOSError(OS_DISPLAY_LOC, err);
	OSSemCreate(&DisplayQ_Sem4, "Display sem4", 0, &err);
	assertOSError(OS_DISPLAY_LOC, err);
	
	UpdateDisplayError_t ret = UpdateDisplay_SetPage(INFO);
	assertUpdateDisplayError(ret);
	return ret;
}

/**
 * @brief Pops the next display message from the queue and passes
 * it to the display driver. Pends on semaphore and mutex to ensure that:
 *  1) queue has messages to send (signaled by semaphore)
 *  2) queue is not currently being written to by a separate thread (mutex)
 * @returns UpdateDisplayError_t
 */
static UpdateDisplayError_t UpdateDisplay_PopNext(){
    DisplayCmd_t cmd;

    OS_ERR err;
    CPU_TS ticks;

    OSSemPend(&DisplayQ_Sem4, 0, OS_OPT_PEND_BLOCKING, &ticks, &err);
    assertOSError(OS_DISPLAY_LOC, err);
		
    OSMutexPend(&DisplayQ_Mutex, 0, OS_OPT_PEND_BLOCKING, &ticks, &err);
    assertOSError(OS_DISPLAY_LOC, err);

    bool result = disp_fifo_get(&msg_queue, &cmd);
    OSMutexPost(&DisplayQ_Mutex, OS_OPT_POST_ALL, &err);
    assertOSError(OS_SEND_CAN_LOC, err);

    if(!result){
			assertUpdateDisplayError(UPDATEDISPLAY_ERR_FIFO_POP);
			return UPDATEDISPLAY_ERR_FIFO_POP;
		}
		
		assertDisplayError(Display_Send(cmd));
		return UPDATEDISPLAY_ERR_NONE;
}

/**
 * @brief Puts a new display message in the queue. Pends on mutex to ensure
 * threadsafe memory access and signals semaphore upon successful fifo_put.
 * @returns UpdateDisplayError_t
 */
static UpdateDisplayError_t UpdateDisplay_PutNext(DisplayCmd_t cmd){
	CPU_TS ticks;
	OS_ERR err;

	OSMutexPend(&DisplayQ_Mutex, 0, OS_OPT_PEND_BLOCKING, &ticks, &err);
  assertOSError(OS_DISPLAY_LOC, err);
	
	bool success = disp_fifo_put(&msg_queue, cmd);

	OSMutexPost(&DisplayQ_Mutex, OS_OPT_POST_ALL, &err);
	assertOSError(OS_DISPLAY_LOC, err);

	if(success){
		OSSemPost(&DisplayQ_Sem4, OS_OPT_POST_ALL, &err);
		assertOSError(OS_DISPLAY_LOC, err);
	}
	else{
		assertUpdateDisplayError(UPDATEDISPLAY_ERR_FIFO_PUT);
		return UPDATEDISPLAY_ERR_FIFO_PUT;
	}

	return UPDATEDISPLAY_ERR_NONE;
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

	UpdateDisplayError_t ret = UpdateDisplay_PutNext(refreshCmd);
	assertUpdateDisplayError(ret);
	return ret;
}

/**
 * @brief Uses component enum to make assigning component values easier.
 * Differentiates between timers, variables, and components to assign values.
 * @param comp component to set value of
 * @param val value
 * @return UpdateDisplayError_t
 */
static UpdateDisplayError_t UpdateDisplay_SetComponent(Component_t comp, uint32_t val){
	UpdateDisplayError_t ret = UPDATEDISPLAY_ERR_NONE;
	
	// For components that are on/off
	if(comp <= MOTOR && val <= 1){
		DisplayCmd_t visCmd = {
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
		
		ret = UpdateDisplay_PutNext(visCmd);
		assertUpdateDisplayError(ret);
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
				{.num=val}
			}
		};

		ret = UpdateDisplay_PutNext(setCmd);
		assertUpdateDisplayError(ret);
		return UpdateDisplay_PutNext(setCmd);
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

	UpdateDisplayError_t ret = UpdateDisplay_PutNext(pgCmd);
	return ret;
}

/* WRAPPERS */
UpdateDisplayError_t UpdateDisplay_SetSOC(uint8_t percent){	// Integer percentage from 0-100
	static uint8_t lastPercent = 0;
	if(percent == lastPercent){
		return UPDATEDISPLAY_ERR_NO_CHANGE;
	}

	UpdateDisplayError_t ret = UpdateDisplay_SetComponent(SOC, percent);
	assertUpdateDisplayError(ret);
	if(ret != UPDATEDISPLAY_ERR_NONE) return ret;

	ret = UpdateDisplay_Refresh();
	assertUpdateDisplayError(ret);
	
	if(ret == UPDATEDISPLAY_ERR_NONE) lastPercent = percent;
	return ret;
}

UpdateDisplayError_t UpdateDisplay_SetSBPV(uint32_t mv){
	static uint32_t lastMv = 0;
	if(mv == lastMv){
		return UPDATEDISPLAY_ERR_NO_CHANGE;
	}

	UpdateDisplayError_t ret = UpdateDisplay_SetComponent(SUPP_BATT, mv/100);
	assertUpdateDisplayError(ret);
	if(ret != UPDATEDISPLAY_ERR_NONE) return ret;

	ret = UpdateDisplay_Refresh();
	assertUpdateDisplayError(ret);

	if(ret == UPDATEDISPLAY_ERR_NONE) lastMv = mv;
	return ret;
}

UpdateDisplayError_t UpdateDisplay_SetVelocity(uint32_t mphTenths){
	static uint32_t lastMphTenths = 0;
	if(mphTenths == lastMphTenths){
		return UPDATEDISPLAY_ERR_NO_CHANGE;
	}
	
	UpdateDisplayError_t ret = UpdateDisplay_SetComponent(VELOCITY, mphTenths);
	assertUpdateDisplayError(ret);
	
	if(ret == UPDATEDISPLAY_ERR_NONE) lastMphTenths = mphTenths;
	return ret;
}

UpdateDisplayError_t UpdateDisplay_SetAccel(uint8_t percent){
	static uint8_t lastPercentAccel = 0;
	if(percent == lastPercentAccel){
		return UPDATEDISPLAY_ERR_NO_CHANGE;
	}

	UpdateDisplayError_t ret = UpdateDisplay_SetComponent(ACCEL_METER, percent);
	assertUpdateDisplayError(ret);
	
	if(ret == UPDATEDISPLAY_ERR_NONE) lastPercentAccel = percent;
	return ret;
}

UpdateDisplayError_t UpdateDisplay_SetArray(bool state){
	static bool lastState = false;
	if(state == lastState){
		return UPDATEDISPLAY_ERR_NO_CHANGE;
	}
	
	UpdateDisplayError_t ret = UpdateDisplay_SetComponent(ARRAY, (state)?1:0);
	assertUpdateDisplayError(ret);
	
	if(ret == UPDATEDISPLAY_ERR_NONE) lastState = state;
	return ret;
}

UpdateDisplayError_t UpdateDisplay_SetMotor(bool state){
	static bool lastState = false;
	if(state == lastState){
		return UPDATEDISPLAY_ERR_NO_CHANGE;
	}
	
	UpdateDisplayError_t ret = UpdateDisplay_SetComponent(MOTOR, (state)?1:0);
	assertUpdateDisplayError(ret);
	
	if(ret == UPDATEDISPLAY_ERR_NONE) lastState = state;
	return ret;
}

UpdateDisplayError_t UpdateDisplay_SetGear(TriState_t gear){
	static TriState_t lastGear = STATE_0;
	if(gear == lastGear){
		return UPDATEDISPLAY_ERR_NO_CHANGE;
	}
	
	UpdateDisplayError_t ret = UpdateDisplay_SetComponent(GEAR, (uint32_t)gear);
	assertUpdateDisplayError(ret);
	if(ret != UPDATEDISPLAY_ERR_NONE) return ret;

	ret = UpdateDisplay_Refresh();
	assertUpdateDisplayError(ret);
	
	if(ret == UPDATEDISPLAY_ERR_NONE) lastGear = gear;
	return ret;
}

UpdateDisplayError_t UpdateDisplay_SetRegenState(TriState_t state){
	static TriState_t lastState = STATE_0;
	if(state == lastState){
		return UPDATEDISPLAY_ERR_NO_CHANGE;
	}
	
	UpdateDisplayError_t ret = UpdateDisplay_SetComponent(REGEN_ST, (uint32_t)state);
	assertUpdateDisplayError(ret);
	if(ret != UPDATEDISPLAY_ERR_NONE) return ret;
	
	ret = UpdateDisplay_Refresh();
	assertUpdateDisplayError(ret);
	
	if(ret == UPDATEDISPLAY_ERR_NONE) lastState = state;
	return ret;
}

UpdateDisplayError_t UpdateDisplay_SetCruiseState(TriState_t state){
	static TriState_t lastState = STATE_0;
	if(state == lastState){
		return UPDATEDISPLAY_ERR_NO_CHANGE;
	}
	
	UpdateDisplayError_t ret = UpdateDisplay_SetComponent(CRUISE_ST, (uint32_t)state);
	if(ret != UPDATEDISPLAY_ERR_NONE) return ret;

	ret = UpdateDisplay_Refresh();
	assertUpdateDisplayError(ret);

	if(ret == UPDATEDISPLAY_ERR_NONE) lastState = state;
	return ret;
}

/**
 * @brief Loops through the display queue and sends all messages
 */
void Task_UpdateDisplay(void *p_arg) {
    while (1) {
			UpdateDisplayError_t err = UpdateDisplay_PopNext();
			assertUpdateDisplayError(err);
    }
}

/*@}*/
