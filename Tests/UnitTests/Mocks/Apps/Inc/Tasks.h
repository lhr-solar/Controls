/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#pragma once
#include_next "Tasks.h"
#ifndef TEST_TASKS

#include "fff.h"

#ifndef TEST_MAIN
DECLARE_FAKE_VOID_FUNC(Task_Init, void *);
#else
void Task_Init(void* p_arg);
#endif

#ifndef TEST_DEBUGDUMP
DECLARE_FAKE_VOID_FUNC(Task_DebugDump, void*);
#else
void Task_DebugDump(void *p_arg);
#endif

#ifndef TEST_COMMANDLINE
DECLARE_FAKE_VOID_FUNC(Task_CommandLine, void*);
#else
void Task_CommandLine(void* p_arg);
#endif

DECLARE_FAKE_VOID_FUNC(TaskSwHookInit);
DECLARE_FAKE_VOID_FUNC(EmergencyContactorOpen);
DECLARE_FAKE_VOID_FUNC(ThrowTaskError, ErrorCode, Callback, ErrorSchedulerLockOpt, ErrorRecovOpt);
DECLARE_FAKE_VOID_FUNC(AssertOsError, OS_ERR);

#endif