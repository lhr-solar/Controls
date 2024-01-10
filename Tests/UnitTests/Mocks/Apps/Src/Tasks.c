#include "fff.h"
#include "Tasks.h"

DEFINE_FAKE_VOID_FUNC(Task_Init);

DEFINE_FAKE_VOID_FUNC(Task_SendTritium);

DEFINE_FAKE_VOID_FUNC(Task_ReadCarCAN);

DEFINE_FAKE_VOID_FUNC(Task_UpdateDisplay);

DEFINE_FAKE_VOID_FUNC(Task_ReadTritium);

DEFINE_FAKE_VOID_FUNC(Task_SendCarCAN);

DEFINE_FAKE_VOID_FUNC(Task_DebugDump);

DEFINE_FAKE_VOID_FUNC(Task_CommandLine);

DEFINE_FAKE_VOID_FUNC(TaskSwHook_Init);

DEFINE_FAKE_VOID_FUNC(EmergencyContactorOpen);

DEFINE_FAKE_VOID_FUNC(_assertOSError);
