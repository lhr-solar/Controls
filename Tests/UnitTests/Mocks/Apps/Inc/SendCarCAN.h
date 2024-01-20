/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#ifdef TEST_SENDCARCAN
#include_next "SendCarCAN.h"
#else

#ifndef __SENDCARCAN_H
#define __SENDCARCAN_H

#include "fff.h"
#include "CANbus.h"

DECLARE_FAKE_VOID_FUNC(Task_SendCarCAN, void*);

DECLARE_FAKE_VOID_FUNC(SendCarCAN_Init);

DECLARE_FAKE_VOID_FUNC(SendCarCAN_Put, CANDATA_t);

#endif
#endif