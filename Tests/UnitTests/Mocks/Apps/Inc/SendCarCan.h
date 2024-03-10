/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#pragma once
#include_next "SendCarCan.h"
#ifndef TEST_SENDCARCAN

#include "fff.h"

DECLARE_FAKE_VOID_FUNC(TaskSendCarCan, void*);
DECLARE_FAKE_VOID_FUNC(SendCarCanInit);
DECLARE_FAKE_VOID_FUNC(SendCarCanPut, CanData);

#endif