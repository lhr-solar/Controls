/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#include "SendCarCan.h"

DEFINE_FAKE_VOID_FUNC(TaskSendCarCan, void*);
DEFINE_FAKE_VOID_FUNC(SendCarCanInit);
DEFINE_FAKE_VOID_FUNC(SendCarCanPut, CanData);