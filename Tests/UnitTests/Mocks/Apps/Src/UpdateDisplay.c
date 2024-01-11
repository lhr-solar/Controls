#include "UpdateDisplay.h"
#include "Minions.h"
#include <math.h>
#include "fff.h"

/**
 * Creates queue for display commands.
 */
#define DISP_Q_SIZE 10

#define FIFO_TYPE DisplayCmd_t
#define FIFO_SIZE DISP_Q_SIZE
#define FIFO_NAME disp_fifo
#include "fifo.h"

// For fault handling
#define RESTART_THRESHOLD 3 // number of times to reset before displaying the fault screen

disp_fifo_t msg_queue;


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

DEFINE_FAKE_VALUE_FUNC(UpdateDisplayError_t, UpdateDisplay_Init);

DEFINE_FAKE_VALUE_FUNC(UpdateDisplayError_t, UpdateDisplay_SetPage, Page_t);

DEFINE_FAKE_VALUE_FUNC(UpdateDisplayError_t, UpdateDisplay_SetSOC, uint8_t);

DEFINE_FAKE_VALUE_FUNC(UpdateDisplayError_t, UpdateDisplay_SetSBPV, uint32_t);

DEFINE_FAKE_VALUE_FUNC(UpdateDisplayError_t, UpdateDisplay_SetVelocity, uint32_t);

DEFINE_FAKE_VALUE_FUNC(UpdateDisplayError_t, UpdateDisplay_SetAccel, uint8_t);

DEFINE_FAKE_VALUE_FUNC(UpdateDisplayError_t, UpdateDisplay_SetArray, bool);

DEFINE_FAKE_VALUE_FUNC(UpdateDisplayError_t, UpdateDisplay_SetMotor, bool);

DEFINE_FAKE_VALUE_FUNC(UpdateDisplayError_t, UpdateDisplay_SetGear, TriState_t);

DEFINE_FAKE_VALUE_FUNC(UpdateDisplayError_t, UpdateDisplay_SetRegenState, TriState_t);

DEFINE_FAKE_VALUE_FUNC(UpdateDisplayError_t, UpdateDisplay_SetCruiseState, TriState_t);

DEFINE_FAKE_VOID_FUNC(UpdateDisplay_ClearQueue);

