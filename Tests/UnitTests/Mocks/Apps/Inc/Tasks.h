#ifdef TEST_READCARCAN
#include_next "ReadCarCAN.h" // Include the next instance of the file. 
// If the real version is in the include search paths after the mock one, it will include it here
#else // Mocked Contactors.h

#ifndef __TASKS_H
#define __TASKS_H
#include "fff.h"

DECLARE_FAKE_VOID_FUNC(Task_Init);

DECLARE_FAKE_VOID_FUNC(Task_SendTritium);

DECLARE_FAKE_VOID_FUNC(Task_ReadCarCAN);

DECLARE_FAKE_VOID_FUNC(Task_UpdateDisplay);

DECLARE_FAKE_VOID_FUNC(Task_ReadTritium);

DECLARE_FAKE_VOID_FUNC(Task_SendCarCAN);

DECLARE_FAKE_VOID_FUNC(Task_DebugDump);

DECLARE_FAKE_VOID_FUNC(Task_CommandLine);

DECLARE_FAKE_VOID_FUNC(TaskSwHook_Init);

DECLARE_FAKE_VOID_FUNC(EmergencyContactorOpen);

DECLARE_FAKE_VOID_FUNC(_assertOSError);
#endif
#endif