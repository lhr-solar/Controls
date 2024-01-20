/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#include "Tasks.h"

#include "ReadTritium.h"
#include "ReadCarCAN.h"
#include "UpdateDisplay.h"

error_code_t Error_ReadCarCAN = READCARCAN_ERR_NONE; // TODO: change this back to the error 
error_code_t Error_ReadTritium = T_NONE;  // Initialized to no error
error_code_t Error_UpdateDisplay = UPDATEDISPLAY_ERR_NONE;

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
DEFINE_FAKE_VOID_FUNC(_assertOSError, OS_ERR);