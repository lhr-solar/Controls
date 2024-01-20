/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#pragma once
#include_next "SendCarCAN.h"
#ifndef TEST_SENDCARCAN

#include "fff.h"

DECLARE_FAKE_VOID_FUNC(Task_SendCarCAN, void*);
DECLARE_FAKE_VOID_FUNC(SendCarCAN_Init);
DECLARE_FAKE_VOID_FUNC(SendCarCAN_Put, CANDATA_t);

#endif