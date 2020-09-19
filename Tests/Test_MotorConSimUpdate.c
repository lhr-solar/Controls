#include "common.h"
#include "config.h"
#include <bsp.h>
#define MOTOR_DRIVE 0x221

int main(){
    uint8_t data[8] = {0x00, 0x11, 0x11, 0x11,     0x22, 0x22, 0x22, 0x22};
    BSP_CAN_Write(CAN_2, MOTOR_DRIVE, data, 8);
}