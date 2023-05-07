#include "common.h"
#include "config.h"
#include <bsp.h>
#define MOTOR_DRIVE 0x221

int main(){            //velocity                   motor current
    uint8_t data[8] = {0x00, 0x11, 0x03, 0xE9,     0x00, 0x00, 0x99, 0x99}; //test torque mode
    BSP_CAN_Write(CAN_2, MOTOR_DRIVE, data, 8);
}