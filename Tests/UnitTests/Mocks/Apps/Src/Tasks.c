/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#include "Tasks.h"

#include "ReadTritium.h"
#include "ReadCarCan.h"
#include "UpdateDisplay.h"

ErrorCode error_read_car_can = kReadCarCanErrNone; 
ErrorCode error_read_tritium = kNone;  // Initialized to no error
ErrorCode error_update_display = kUpdateDisplayErrNone;

#ifndef TEST_MAIN
DEFINE_FAKE_VOID_FUNC(TaskInit, void*);
#endif

#ifndef TEST_DEBUGDUMP
DEFINE_FAKE_VOID_FUNC(TaskDebugDump, void*);
#endif

#ifndef TEST_COMMANDLINE
DEFINE_FAKE_VOID_FUNC(TaskCommandLine, void*);
#endif

DEFINE_FAKE_VOID_FUNC(TaskSwHookInit);
DEFINE_FAKE_VOID_FUNC(EmergencyContactorOpen);
DEFINE_FAKE_VOID_FUNC(ThrowTaskError, ErrorCode, Callback, ErrorSchedulerLockOpt, ErrorRecovOpt);
DEFINE_FAKE_VOID_FUNC(AssertOsError, OS_ERR);