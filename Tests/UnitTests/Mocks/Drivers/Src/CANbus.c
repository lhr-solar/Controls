#include "fff.h"
#include "CANbus.h"

DEFINE_FAKE_VALUE_FUNC(ErrorStatus, CANbus_Init, CAN_t, CANId_t*, uint8_t);

DEFINE_FAKE_VALUE_FUNC(ErrorStatus, CANbus_Send, CANDATA_t, bool, CAN_t);

DEFINE_FAKE_VALUE_FUNC(ErrorStatus, CANbus_Read, CANDATA_t*, bool, CAN_t);