#ifndef __IOSTATE_H
#define __IOSTATE_H

#include "CANbus.h"

// Macros for the SWITCH_BITMAP signal in the IO_STATE CAN message
#define SWITCH_BITMAP_BRAKELIGHT(value)       ((value & 0x01) << 7) // Bit 7: BRAKELIGHT
#define SWITCH_BITMAP_CRUZ_ST(value)          ((value & 0x01) << 6) // Bit 6: CRUZ_ST
#define SWITCH_BITMAP_CRUZ_EN(value)          ((value & 0x01) << 5) // Bit 5: CRUZ_EN
#define SWITCH_BITMAP_REV_SW(value)           ((value & 0x01) << 4) // Bit 4: REV_SW
#define SWITCH_BITMAP_FOR_SW(value)           ((value & 0x01) << 3) // Bit 3: FOR_SW
#define SWITCH_BITMAP_REGEN_SW(value)         ((value & 0x01) << 2) // Bit 2: REGEN_SW
#define SWITCH_BITMAP_IGN_2_MOTOR(value)      ((value & 0x01) << 1) // Bit 1: IGN_2_Motor
#define SWITCH_BITMAP_IGN_1_ARRAY(value)      ((value & 0x01) << 0) // Bit 0: IGN_1_Array

typedef enum {
    IOSTATE_ERR_NONE  = 0x0000,         // No error
    IOSTATE_ERROR = 0xFFFF,             // More than one state is high at a time
} IOState_error_code_t;

void assertIOStateError(IOState_error_code_t io_err);

#endif