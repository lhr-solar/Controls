/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#include "SendCarCan.h"

DEFINE_FAKE_VOID_FUNC(Task_SendCarCAN, void*);
DEFINE_FAKE_VOID_FUNC(SendCarCanInit);
DEFINE_FAKE_VOID_FUNC(SendCarCanPut, CanData);