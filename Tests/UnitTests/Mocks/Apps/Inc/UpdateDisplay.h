//////////////////////////// MOCK
#ifdef TEST_UPDATEDISPLAY
#include_next "UpdateDisplay.h"
#else
#ifndef __UPDATE_DISPLAY_H
#define __UPDATE_DISPLAY_H

//#include "os.h"
#include "common.h"
//#include "Tasks.h"

#include "Display.h"
//#include "Contactors.h"
#include "fff.h"

/**
 * Error types
 */
typedef enum{
	UPDATEDISPLAY_ERR_NONE,
	UPDATEDISPLAY_ERR_FIFO_PUT,     // Error putting command in fifo
	UPDATEDISPLAY_ERR_FIFO_POP,     // Error popping command from fifo
	UPDATEDISPLAY_ERR_PARSE_COMP,   // Error parsing component/val in SetComponent
	UPDATEDISPLAY_ERR_DRIVER        // Driver call returned an error
} UpdateDisplayError_t;

/**
 * For display elements with three states
 */
typedef enum{
	STATE_0	=0,
	STATE_1	=1,
	STATE_2	=2
} TriState_t;

// For cruise control and regen
#define DISP_DISABLED STATE_0
#define DISP_ENABLED STATE_1	// Able to be used
#define DISP_ACTIVE STATE_2	// Actively being used right now

// For gear changes
#define DISP_NEUTRAL STATE_0
#define DISP_FORWARD STATE_1
#define DISP_REVERSE STATE_2

DECLARE_FAKE_VALUE_FUNC(UpdateDisplayError_t, UpdateDisplay_Init);

DECLARE_FAKE_VALUE_FUNC(UpdateDisplayError_t, UpdateDisplay_SetPage, Page_t);

DECLARE_FAKE_VALUE_FUNC(UpdateDisplayError_t, UpdateDisplay_SetSOC, uint8_t);

DECLARE_FAKE_VALUE_FUNC(UpdateDisplayError_t, UpdateDisplay_SetSBPV, uint32_t);

DECLARE_FAKE_VALUE_FUNC(UpdateDisplayError_t, UpdateDisplay_SetVelocity, uint32_t);

DECLARE_FAKE_VALUE_FUNC(UpdateDisplayError_t, UpdateDisplay_SetAccel, uint8_t);

DECLARE_FAKE_VALUE_FUNC(UpdateDisplayError_t, UpdateDisplay_SetArray, bool);

DECLARE_FAKE_VALUE_FUNC(UpdateDisplayError_t, UpdateDisplay_SetMotor, bool);

DECLARE_FAKE_VALUE_FUNC(UpdateDisplayError_t, UpdateDisplay_SetGear, TriState_t);

DECLARE_FAKE_VALUE_FUNC(UpdateDisplayError_t, UpdateDisplay_SetRegenState, TriState_t);

DECLARE_FAKE_VALUE_FUNC(UpdateDisplayError_t, UpdateDisplay_SetCruiseState, TriState_t);

DECLARE_FAKE_VOID_FUNC(UpdateDisplay_ClearQueue);
#endif
#endif