/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////
#include "fff.h"
#include "Tasks.h"
#include "Minions.h"
#include "ReadCarCAN.h"
#include "UpdateDisplay.h"
#include "ReadTritium.h"

/**
 * TCBs
 */
OS_TCB Init_TCB;
OS_TCB SendTritium_TCB;
OS_TCB ReadCarCAN_TCB;
OS_TCB UpdateDisplay_TCB;
OS_TCB ReadTritium_TCB;
OS_TCB SendCarCAN_TCB;
OS_TCB DebugDump_TCB;
OS_TCB CommandLine_TCB;

error_code_t Error_ReadCarCAN = READCARCAN_ERR_NONE; 
error_code_t Error_ReadTritium = T_NONE;  
error_code_t Error_UpdateDisplay = UPDATEDISPLAY_ERR_NONE;

extern const pinInfo_t PININFO_LUT[]; // For GPIO writes. Externed from Minions Driver C file.

#ifndef TEST_MAIN
DEFINE_FAKE_VOID_FUNC(Task_Init, void*);
#endif

#ifndef TEST_DEBUGDUMP
DEFINE_FAKE_VOID_FUNC(Task_DebugDump, void*);
#endif

#ifndef TEST_COMMANDLINE
DEFINE_FAKE_VOID_FUNC(Task_CommandLine, void*);
#endif

DEFINE_FAKE_VOID_FUNC(TaskSwHook_Init);

DEFINE_FAKE_VOID_FUNC(EmergencyContactorOpen);

DEFINE_FAKE_VOID_FUNC(throwTaskError, error_code_t, callback_t, error_scheduler_lock_opt_t, error_recov_opt_t);

DEFINE_FAKE_VOID_FUNC(assertOSError, OS_ERR);
