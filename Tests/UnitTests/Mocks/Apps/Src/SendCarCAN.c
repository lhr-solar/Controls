/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#include "SendCarCAN.h"

DEFINE_FAKE_VOID_FUNC(Task_SendCarCAN, void*);
DEFINE_FAKE_VOID_FUNC(SendCarCAN_Init);
DEFINE_FAKE_VOID_FUNC(SendCarCAN_Put, CANDATA_t);