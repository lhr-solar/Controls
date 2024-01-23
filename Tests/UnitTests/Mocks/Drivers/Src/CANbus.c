/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#include "CANbus.h"

DEFINE_FAKE_VALUE_FUNC(ErrorStatus, CanBusInit, Can, CanId*, uint8_t);
DEFINE_FAKE_VALUE_FUNC(ErrorStatus, CanBusSend, CanData, bool, Can);
DEFINE_FAKE_VALUE_FUNC(ErrorStatus, CanBusRead, CanData*, bool, Can);