/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#include "Tasks.h"

#include "ReadTritium.h"
#include "ReadCarCan.h"
#include "UpdateDisplay.h"

// ErrorCode Error_ReadCarCAN = READCARCAN_ERR_NONE; // TODO: change this back to the error 
// ErrorCode Error_ReadTritium = T_NONE;  // Initialized to no error
// ErrorCode Error_UpdateDisplay = UPDATEDISPLAY_ERR_NONE;

#ifndef TEST_MAIN
DEFINE_FAKE_VOID_FUNC(Task_Init, void*);
#endif

#ifndef TEST_DEBUGDUMP
DEFINE_FAKE_VOID_FUNC(Task_DebugDump, void*);
#endif

#ifndef TEST_COMMANDLINE
DEFINE_FAKE_VOID_FUNC(Task_CommandLine, void*);
#endif

DEFINE_FAKE_VOID_FUNC(TaskSwHookInit);
DEFINE_FAKE_VOID_FUNC(EmergencyContactorOpen);
DEFINE_FAKE_VOID_FUNC(ThrowTaskError, ErrorCode, Callback, ErrorSchedulerLockOpt, ErrorRecovOpt);
DEFINE_FAKE_VOID_FUNC(AssertOsError, OS_ERR);